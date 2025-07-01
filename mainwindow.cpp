#include "mainwindow.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_processor(nullptr)
    , m_processingTimer(new QTimer(this))
{
    setupUI();

    m_processor = new FileProcessor(this);
    connect(m_processor, &FileProcessor::finished, this, &MainWindow::onProcessingFinished);
    connect(m_processor, &FileProcessor::progressUpdated, this, &MainWindow::onProgressUpdate);
    connect(m_processor, &FileProcessor::statusUpdated, this, &MainWindow::onStatusUpdate);
    connect(m_processor, &FileProcessor::errorOccurred, this, &MainWindow::onErrorOccurred);

    connect(m_processingTimer, &QTimer::timeout, this, [this]() {
        if (!m_processor->isRunning()) {
            startProcessing();
        }
    });

    m_inputPathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    m_outputPathEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/processed");
    m_fileMaskEdit->setText("*.txt");
    m_timerIntervalSpin->setValue(5);
    m_xorValueEdit->setText("0123456789ABCDEF");

    m_overwriteRadio->setChecked(true);
    m_onceModeRadio->setChecked(true);

    m_stopBtn->setEnabled(false);
}

MainWindow::~MainWindow()
{
    if (m_processor && m_processor->isRunning()) {
        m_processor->stop();
        m_processor->wait();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);

    createInputGroup();
    createOutputGroup();
    createProcessingGroup();
    createControlGroup();
    createStatusGroup();

    setWindowTitle("File Processor v1.0");
    resize(600, 700);
}

void MainWindow::createInputGroup()
{
    m_inputGroup = new QGroupBox("Входные файлы", this);
    QGridLayout *layout = new QGridLayout(m_inputGroup);

    layout->addWidget(new QLabel("Путь к входным файлам:"), 0, 0);
    m_inputPathEdit = new QLineEdit;
    layout->addWidget(m_inputPathEdit, 0, 1);
    m_browseInputBtn = new QPushButton("Обзор...");
    layout->addWidget(m_browseInputBtn, 0, 2);
    connect(m_browseInputBtn, &QPushButton::clicked, this, &MainWindow::browseInputPath);

    layout->addWidget(new QLabel("Маска файлов:"), 1, 0);
    m_fileMaskEdit = new QLineEdit;
    layout->addWidget(m_fileMaskEdit, 1, 1, 1, 2);

    m_deleteInputCheck = new QCheckBox("Удалять входные файлы после обработки");
    layout->addWidget(m_deleteInputCheck, 2, 0, 1, 3);

    m_mainLayout->addWidget(m_inputGroup);
}

void MainWindow::createOutputGroup()
{
    m_outputGroup = new QGroupBox("Выходные файлы", this);
    QGridLayout *layout = new QGridLayout(m_outputGroup);

    layout->addWidget(new QLabel("Путь для сохранения:"), 0, 0);
    m_outputPathEdit = new QLineEdit;
    layout->addWidget(m_outputPathEdit, 0, 1);
    m_browseOutputBtn = new QPushButton("Обзор...");
    layout->addWidget(m_browseOutputBtn, 0, 2);
    connect(m_browseOutputBtn, &QPushButton::clicked, this, &MainWindow::browseOutputPath);

    layout->addWidget(new QLabel("При совпадении имени:"), 1, 0);
    m_fileExistsGroup = new QButtonGroup(this);
    m_overwriteRadio = new QRadioButton("Перезаписать");
    m_modifyNameRadio = new QRadioButton("Изменить имя (добавить счетчик)");
    m_fileExistsGroup->addButton(m_overwriteRadio);
    m_fileExistsGroup->addButton(m_modifyNameRadio);

    QHBoxLayout *radioLayout = new QHBoxLayout;
    radioLayout->addWidget(m_overwriteRadio);
    radioLayout->addWidget(m_modifyNameRadio);
    layout->addLayout(radioLayout, 1, 1, 1, 2);

    m_mainLayout->addWidget(m_outputGroup);
}

void MainWindow::createProcessingGroup()
{
    m_processingGroup = new QGroupBox("Настройки обработки", this);
    QGridLayout *layout = new QGridLayout(m_processingGroup);

    layout->addWidget(new QLabel("Режим работы:"), 0, 0);
    m_modeGroup = new QButtonGroup(this);
    m_onceModeRadio = new QRadioButton("Разовый запуск");
    m_timerModeRadio = new QRadioButton("По таймеру");
    m_modeGroup->addButton(m_onceModeRadio);
    m_modeGroup->addButton(m_timerModeRadio);

    QHBoxLayout *modeLayout = new QHBoxLayout;
    modeLayout->addWidget(m_onceModeRadio);
    modeLayout->addWidget(m_timerModeRadio);
    layout->addLayout(modeLayout, 0, 1, 1, 2);

    m_timerLabel = new QLabel("Интервал опроса (сек):");
    layout->addWidget(m_timerLabel, 1, 0);
    m_timerIntervalSpin = new QSpinBox;
    m_timerIntervalSpin->setRange(1, 3600);
    m_timerIntervalSpin->setSuffix(" сек");
    layout->addWidget(m_timerIntervalSpin, 1, 1);

    connect(m_timerModeRadio, &QRadioButton::toggled, m_timerLabel, &QLabel::setEnabled);
    connect(m_timerModeRadio, &QRadioButton::toggled, m_timerIntervalSpin, &QSpinBox::setEnabled);

    layout->addWidget(new QLabel("Значение для XOR (8 байт):"), 2, 0);
    m_xorValueEdit = new QLineEdit;
    m_xorValueEdit->setMaxLength(16);
    layout->addWidget(m_xorValueEdit, 2, 1, 1, 2);
    connect(m_xorValueEdit, &QLineEdit::textChanged, this, &MainWindow::validateXorValue);

    m_xorHintLabel = new QLabel("Введите 16 HEX символов (0-9, A-F)");
    m_xorHintLabel->setStyleSheet("color: gray; font-size: 10px;");
    layout->addWidget(m_xorHintLabel, 3, 1, 1, 2);

    m_mainLayout->addWidget(m_processingGroup);
}

void MainWindow::createControlGroup()
{
    m_controlGroup = new QGroupBox("Управление", this);
    QHBoxLayout *layout = new QHBoxLayout(m_controlGroup);

    m_startBtn = new QPushButton("Запустить");
    m_stopBtn = new QPushButton("Остановить");

    m_startBtn->setMinimumHeight(40);
    m_stopBtn->setMinimumHeight(40);

    layout->addWidget(m_startBtn);
    layout->addWidget(m_stopBtn);

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::startProcessing);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::stopProcessing);

    m_mainLayout->addWidget(m_controlGroup);
}

void MainWindow::createStatusGroup()
{
    m_statusGroup = new QGroupBox("Статус", this);
    QVBoxLayout *layout = new QVBoxLayout(m_statusGroup);

    m_progressBar = new QProgressBar;
    layout->addWidget(m_progressBar);

    m_statusLabel = new QLabel("Готов к работе");
    layout->addWidget(m_statusLabel);

    m_logEdit = new QTextEdit;
    m_logEdit->setMaximumHeight(150);
    m_logEdit->setReadOnly(true);
    layout->addWidget(m_logEdit);

    m_mainLayout->addWidget(m_statusGroup);
}

void MainWindow::browseInputPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с входными файлами",
                                                    m_inputPathEdit->text());
    if (!dir.isEmpty()) {
        m_inputPathEdit->setText(dir);
    }
}

void MainWindow::browseOutputPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения результатов",
                                                    m_outputPathEdit->text());
    if (!dir.isEmpty()) {
        m_outputPathEdit->setText(dir);
    }
}

void MainWindow::startProcessing()
{
    if (!validateSettings()) {
        return;
    }

    FileProcessorSettings settings;
    settings.inputPath = m_inputPathEdit->text();
    settings.outputPath = m_outputPathEdit->text();
    settings.fileMask = m_fileMaskEdit->text();
    settings.deleteInputFiles = m_deleteInputCheck->isChecked();
    settings.overwriteOutput = m_overwriteRadio->isChecked();

    QString xorText = m_xorValueEdit->text().toUpper();
    bool ok;
    settings.xorValue = xorText.toULongLong(&ok, 16);

    m_processor->setSettings(settings);

    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);

    m_progressBar->setValue(0);
    m_statusLabel->setText("Запуск обработки...");

    if (m_timerModeRadio->isChecked()) {
        m_processingTimer->start(m_timerIntervalSpin->value() * 1000);
        m_logEdit->append(QString("[%1] Запущен режим по таймеру (интервал: %2 сек)")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(m_timerIntervalSpin->value()));
    }

    m_processor->start();
}

void MainWindow::stopProcessing()
{
    m_processingTimer->stop();

    if (m_processor->isRunning()) {
        m_processor->stop();
        m_statusLabel->setText("Остановка обработки...");
    } else {
        onProcessingFinished();
    }
}

void MainWindow::onProcessingFinished()
{
    m_startBtn->setEnabled(true);

    if (!m_timerModeRadio->isChecked()) {
        m_statusLabel->setText("Обработка завершена");
    } else if (!m_processingTimer->isActive()) {
        m_statusLabel->setText("Остановлено");
    }
}

void MainWindow::onProgressUpdate(int progress)
{
    m_progressBar->setValue(progress);
}

void MainWindow::onStatusUpdate(const QString &status)
{
    m_statusLabel->setText(status);
    m_logEdit->append(QString("[%1] %2")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                          .arg(status));
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_logEdit->append(QString("[%1] ОШИБКА: %2")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                          .arg(error));
    QMessageBox::warning(this, "Ошибка", error);
}

bool MainWindow::validateSettings()
{
    if (m_inputPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите путь к входным файлам");
        return false;
    }

    if (m_outputPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите путь для сохранения результатов");
        return false;
    }

    if (!QDir(m_inputPathEdit->text()).exists()) {
        QMessageBox::warning(this, "Ошибка", "Папка с входными файлами не существует");
        return false;
    }

    if (m_fileMaskEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите маску файлов");
        return false;
    }

    QString xorError = getXorValueError();
    if (!xorError.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", xorError);
        return false;
    }

    return true;
}

QString MainWindow::getXorValueError()
{
    QString text = m_xorValueEdit->text().trimmed();

    if (text.isEmpty()) {
        return "Введите значение для XOR операции";
    }

    if (text.length() != 16) {
        return "Значение XOR должно содержать ровно 16 символов";
    }

    for (QChar c : text) {
        if (!c.isDigit() && !(c >= 'A' && c <= 'F') && !(c >= 'a' && c <= 'f')) {
            return "Значение XOR должно содержать только HEX символы (0-9, A-F)";
        }
    }

    return QString();
}

void MainWindow::validateXorValue()
{
    QString error = getXorValueError();
    if (error.isEmpty()) {
        m_xorHintLabel->setText("✓ Корректное значение");
        m_xorHintLabel->setStyleSheet("color: green; font-size: 10px;");
    } else {
        m_xorHintLabel->setText(error);
        m_xorHintLabel->setStyleSheet("color: red; font-size: 10px;");
    }
}
