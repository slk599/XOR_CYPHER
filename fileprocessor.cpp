#include "fileprocessor.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

FileProcessor::FileProcessor(QObject *parent)
    : QThread(parent)
    , m_stopRequested(false)
{
}

void FileProcessor::setSettings(const FileProcessorSettings &settings)
{
    m_settings = settings;
}

void FileProcessor::stop()
{
    m_stopRequested = true;
}

void FileProcessor::run()
{
    m_stopRequested = false;

    emit statusUpdated("Поиск файлов для обработки...");

    QStringList inputFiles = findInputFiles();

    if (inputFiles.isEmpty()) {
        emit statusUpdated("Файлы для обработки не найдены");
        return;
    }

    emit statusUpdated(QString("Найдено файлов: %1").arg(inputFiles.size()));

    QDir outputDir(m_settings.outputPath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            emit errorOccurred("Не удалось создать выходную папку: " + m_settings.outputPath);
            return;
        }
    }

    int processedCount = 0;
    int totalCount = inputFiles.size();

    for (const QString &inputFile : inputFiles) {
        if (m_stopRequested) {
            emit statusUpdated("Обработка прервана пользователем");
            break;
        }

        QFileInfo fileInfo(inputFile);
        QString outputFileName = fileInfo.fileName();
        QString outputFilePath = outputDir.absoluteFilePath(outputFileName);

        if (!m_settings.overwriteOutput && QFile::exists(outputFilePath)) {
            outputFilePath = generateUniqueFileName(outputFilePath);
        }

        emit statusUpdated(QString("Обработка: %1").arg(fileInfo.fileName()));

        if (processFile(inputFile, outputFilePath)) {
            processedCount++;

            if (m_settings.deleteInputFiles) {
                if (!QFile::remove(inputFile)) {
                    emit errorOccurred(QString("Не удалось удалить входной файл: %1").arg(inputFile));
                }
            }
        } else {
            emit errorOccurred(QString("Не удалось обработать файл: %1").arg(inputFile));
        }

        int progress = (processedCount * 100) / totalCount;
        emit progressUpdated(progress);
    }

    if (!m_stopRequested) {
        emit statusUpdated(QString("Обработка завершена. Обработано файлов: %1 из %2")
                               .arg(processedCount).arg(totalCount));
        emit progressUpdated(100);
    }
}

bool FileProcessor::processFile(const QString &inputFilePath, const QString &outputFilePath)
{
    QFile inputFile(inputFilePath);
    QFile outputFile(outputFilePath);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (!outputFile.open(QIODevice::WriteOnly)) {
        inputFile.close();
        return false;
    }

    qint64 totalSize = inputFile.size();
    qint64 processedSize = 0;

    char *buffer = new char[BUFFER_SIZE];

    while (!inputFile.atEnd() && !m_stopRequested) {
        qint64 bytesRead = inputFile.read(buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            break;
        }

        xorProcessBuffer(buffer, bytesRead);

        qint64 bytesWritten = outputFile.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            delete[] buffer;
            inputFile.close();
            outputFile.close();
            return false;
        }

        processedSize += bytesRead;

        if (totalSize > 10 * 1024 * 1024) {
            int fileProgress = (processedSize * 100) / totalSize;
        }
    }

    delete[] buffer;
    inputFile.close();
    outputFile.close();

    return !m_stopRequested;
}

QString FileProcessor::generateUniqueFileName(const QString &basePath)
{
    QFileInfo fileInfo(basePath);
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    QString dirPath = fileInfo.absolutePath();

    int counter = 1;
    QString newFileName;

    do {
        if (extension.isEmpty()) {
            newFileName = QString("%1_%2").arg(baseName).arg(counter);
        } else {
            newFileName = QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension);
        }
        counter++;
    } while (QFile::exists(QDir(dirPath).absoluteFilePath(newFileName)));

    return QDir(dirPath).absoluteFilePath(newFileName);
}

QStringList FileProcessor::findInputFiles()
{
    QDir inputDir(m_settings.inputPath);

    QStringList masks = m_settings.fileMask.split(';', Qt::SkipEmptyParts);
    for (QString &mask : masks) {
        mask = mask.trimmed();
    }

    if (masks.isEmpty()) {
        masks << "*";
    }

    inputDir.setNameFilters(masks);
    inputDir.setFilter(QDir::Files | QDir::Readable);

    QStringList files;
    QFileInfoList fileInfoList = inputDir.entryInfoList();

    for (const QFileInfo &fileInfo : fileInfoList) {
        files << fileInfo.absoluteFilePath();
    }

    return files;
}

void FileProcessor::xorProcessBuffer(char *buffer, qint64 size)
{
    union {
        quint64 value;
        char bytes[8];
    } xorKey;

    xorKey.value = m_settings.xorValue;

    for (qint64 i = 0; i < size; ++i) {
        buffer[i] ^= xorKey.bytes[i % 8];
    }
}
