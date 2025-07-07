#include "mainwindow.h"
#include "cameraregistrationdialog.h"
#include "loghistorydialog.h"
#include <QApplication>
#include <QHeaderView>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentCameraNumber(1)
{
    setupUI();
    setWindowTitle("Smart SafetyNet");
    setMinimumSize(1000, 650);

    // ✅ 다크 테마 적용
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2b2b;
            color: white;
        }
        QLabel {
            color: white;
        }
        QLineEdit {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 5px;
            border-radius: 4px;
        }
        QComboBox {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 5px;
            border-radius: 4px;
        }
        QTableWidget {
            background-color: #404040;
            color: white;
            gridline-color: #555;
            alternate-background-color: #353535;
        }
        QHeaderView::section {
            background-color: #353535;
            color: white;
            font-weight: bold;
            padding: 6px;
            border: 1px solid #555;
        }
        QPushButton {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 6px 12px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QCheckBox {
            color: white;
        }
    )");

    registeredCameras << "Camera 1 (192.168.1.100)" << "Camera 2 (192.168.1.101)";
    cameraSelectCombo->addItems(registeredCameras);

    addLogEntry("Camera 1", "Motion detected");
    addLogEntry("Camera 2", "PPE violation");
    addLogEntry("Camera 1", "Normal operation");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    setMinimumSize(1100, 700);

    // ✅ 최상단 인사 및 종료 버튼
    QLabel *greetingLabel = new QLabel("Hello admin!");
    greetingLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");

    QPushButton *exitButton = new QPushButton("종료");
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(greetingLabel);
    topLayout->addStretch();
    topLayout->addWidget(exitButton);

    // ✅ 상단 레이블 (비디오 스트리밍 / 로그)
    videoStreamingLabel = new QLabel("Video Streaming");
    videoStreamingLabel->setStyleSheet("font-weight: bold; color: orange;");
    QLabel *logLabel = new QLabel("Safety Alerts");
    logLabel->setStyleSheet("font-weight: bold; color: orange;");

    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addWidget(videoStreamingLabel, 2);
    labelLayout->addWidget(logLabel, 1);

    // ✅ 비디오 스트리밍
    videoWidget = new QVideoWidget();
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);
    videoWidget->setMinimumSize(640, 480);

    // ✅ 로그 테이블
    logTable = new QTableWidget();
    logTable->setColumnCount(2);
    logTable->setHorizontalHeaderLabels({"Camera", "Alert"});
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    logTable->setShowGrid(true);
    logTable->verticalHeader()->setVisible(false);

    QPushButton *logHistoryButton = new QPushButton("전체 로그 보기");
    connect(logHistoryButton, &QPushButton::clicked, this, &MainWindow::onLogHistoryClicked);

    QVBoxLayout *logLayout = new QVBoxLayout();
    logLayout->addWidget(logTable);
    logLayout->addWidget(logHistoryButton);

    // ✅ 비디오 & 로그 수평 배치 (2:1)
    mainLayout = new QHBoxLayout();  // ✅ 초기화 누락된 부분
    mainLayout->addWidget(videoWidget);
    mainLayout->addLayout(logLayout);
    mainLayout->setStretch(0, 2);
    mainLayout->setStretch(1, 1);

    // ✅ 하단 제어 영역
    cameraRegistrationButton = new QPushButton("카메라 등록");
    connect(cameraRegistrationButton, &QPushButton::clicked, this, &MainWindow::onCameraRegistrationClicked);

    cameraSelectCombo = new QComboBox();

    QHBoxLayout *cameraControlRow = new QHBoxLayout();
    cameraControlRow->addWidget(cameraRegistrationButton);
    cameraControlRow->addWidget(cameraSelectCombo);

    QVBoxLayout *leftControlLayout = new QVBoxLayout();
    leftControlLayout->addLayout(cameraControlRow);

    // ✅ 체크박스 수직 정렬
    ppeDetectorCheckBox = new QCheckBox("PPE Detector");
    connect(ppeDetectorCheckBox, &QCheckBox::toggled, this, &MainWindow::onPPEDetectorToggled);

    mosaicerCheckBox = new QCheckBox("Mosaicer");
    connect(mosaicerCheckBox, &QCheckBox::toggled, this, &MainWindow::onMosaicerToggled);

    QVBoxLayout *rightControlLayout = new QVBoxLayout();
    rightControlLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    rightControlLayout->addWidget(ppeDetectorCheckBox);
    rightControlLayout->addWidget(mosaicerCheckBox);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addLayout(leftControlLayout);
    bottomLayout->addLayout(rightControlLayout);
    bottomLayout->setStretch(0, 2);
    bottomLayout->setStretch(1, 1);

    // ✅ 최종 전체 구성
    outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->addLayout(topLayout);
    outerLayout->addLayout(labelLayout);
    outerLayout->addLayout(mainLayout);      // ✅ 누락되었던 mainLayout 포함
    outerLayout->addSpacing(10);
    outerLayout->addLayout(bottomLayout);
}

void MainWindow::setupVideoStreamingSection() {}
void MainWindow::setupLogSection() {}

void MainWindow::addLogEntry(const QString &camera, const QString &alert)
{
    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(camera));
    logTable->setItem(0, 1, new QTableWidgetItem(alert));

    const int maxRows = 20;
    if (logTable->rowCount() > maxRows) {
        logTable->removeRow(logTable->rowCount() - 1);
    }
}

void MainWindow::onCameraRegistrationClicked()
{
    CameraRegistrationDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString cameraInfo = dialog.getCameraInfo();
        registeredCameras.append(cameraInfo);
        cameraSelectCombo->addItem(cameraInfo);
        addLogEntry("System", "New camera registered: " + cameraInfo);
    }
}

void MainWindow::onCameraSelectChanged(const QString &cameraName)
{
    if (cameraName != "Camera Select" && !cameraName.isEmpty()) {
        int cameraIndex = registeredCameras.indexOf(cameraName) + 1;
        currentCameraNumber = cameraIndex;
        cameraNumberLabel->setText(QString("Camera #%1").arg(currentCameraNumber));
        addLogEntry("System", QString("Switched to %1").arg(cameraName));

        if (cameraName.contains("192.168.1.100"))
            mediaPlayer->setSource(QUrl("rtsp://192.168.1.100/stream"));
        else if (cameraName.contains("192.168.1.101"))
            mediaPlayer->setSource(QUrl("rtsp://192.168.1.101/stream"));

        mediaPlayer->play();
    }
}

void MainWindow::onLogHistoryClicked()
{
    LogHistoryDialog dialog(this);
    dialog.exec();
}

void MainWindow::onMosaicerToggled(bool enabled)
{
    if (enabled && ppeDetectorCheckBox->isChecked()) {
        ppeDetectorCheckBox->setChecked(false);
    }
    QString status = enabled ? "enabled" : "disabled";
    addLogEntry("System", QString("Mosaicer %1").arg(status));
}

void MainWindow::onPPEDetectorToggled(bool enabled)
{
    if (enabled && mosaicerCheckBox->isChecked()) {
        mosaicerCheckBox->setChecked(false);
    }
    QString status = enabled ? "enabled" : "disabled";
    addLogEntry("System", QString("PPE Detector %1").arg(status));
}
