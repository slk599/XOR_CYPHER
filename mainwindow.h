#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QTimer>
#include <QButtonGroup>
#include "fileprocessor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void browseInputPath();
    void browseOutputPath();
    void startProcessing();
    void stopProcessing();
    void onProcessingFinished();
    void onProgressUpdate(int progress);
    void onStatusUpdate(const QString &status);
    void onErrorOccurred(const QString &error);
    void validateXorValue();

private:
    void setupUI();
    void createInputGroup();
    void createOutputGroup();
    void createProcessingGroup();
    void createControlGroup();
    void createStatusGroup();

    bool validateSettings();
    QString getXorValueError();

    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;

    QGroupBox *m_inputGroup;
    QLineEdit *m_inputPathEdit;
    QPushButton *m_browseInputBtn;
    QLineEdit *m_fileMaskEdit;
    QCheckBox *m_deleteInputCheck;

    QGroupBox *m_outputGroup;
    QLineEdit *m_outputPathEdit;
    QPushButton *m_browseOutputBtn;
    QButtonGroup *m_fileExistsGroup;
    QRadioButton *m_overwriteRadio;
    QRadioButton *m_modifyNameRadio;

    QGroupBox *m_processingGroup;
    QButtonGroup *m_modeGroup;
    QRadioButton *m_timerModeRadio;
    QRadioButton *m_onceModeRadio;
    QSpinBox *m_timerIntervalSpin;
    QLabel *m_timerLabel;
    QLineEdit *m_xorValueEdit;
    QLabel *m_xorHintLabel;

    QGroupBox *m_controlGroup;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;

    QGroupBox *m_statusGroup;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTextEdit *m_logEdit;

    FileProcessor *m_processor;
    QTimer *m_processingTimer;
};

#endif // MAINWINDOW_H
