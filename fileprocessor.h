#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QThread>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>

struct FileProcessorSettings
{
    QString inputPath;
    QString outputPath;
    QString fileMask;
    bool deleteInputFiles = false;
    bool overwriteOutput = true;
    quint64 xorValue = 0;
};

class FileProcessor : public QThread
{
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);

    void setSettings(const FileProcessorSettings &settings);
    void stop();

signals:
    void progressUpdated(int progress);
    void statusUpdated(const QString &status);
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    bool processFile(const QString &inputFilePath, const QString &outputFilePath);
    QString generateUniqueFileName(const QString &basePath);
    QStringList findInputFiles();
    void xorProcessBuffer(char *buffer, qint64 size);

    FileProcessorSettings m_settings;
    bool m_stopRequested;

    static const qint64 BUFFER_SIZE = 1024 * 1024;
};

#endif // FILEPROCESSOR_H
