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

    registeredCameras << "Camera 1 (192.168.1.100)" << "Camera 2 (192.168.1.101)";
    cameraSelectCombo->addItems(registeredCameras);

    addLogEntry("Camera 1", "Motion detected");
    addLogEntry("Camera 2", "PPE violation");
    addLogEntry("Camera 1", "Normal operation");
}

MainWindow::~MainWindow() {}
/*
void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ✅ 최상단 인사 및 종료 버튼
    QLabel *greetingLabel = new QLabel("Hello admin!");
    greetingLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");
    QPushButton *exitButton = new QPushButton("종료");
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(greetingLabel);
    topLayout->addStretch();
    topLayout->addWidget(exitButton);

    // ✅ 상단 - 영상 + 로그 제목 (2:1 비율)
    videoStreamingLabel = new QLabel("Video Streaming");
    videoStreamingLabel->setStyleSheet("font-weight: bold; color: orange;");
    QLabel *logLabel = new QLabel("Safety Alerts");
    logLabel->setStyleSheet("font-weight: bold; color: orange;");

    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addWidget(videoStreamingLabel, 2);
    labelLayout->addWidget(logLabel, 1);

    // ✅ 영상 위젯
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

    // ✅ 영상+로그를 좌우로 2:1로 나누기
    QHBoxLayout *streamAndLogLayout = new QHBoxLayout();
    streamAndLogLayout->addWidget(videoWidget, 2);
    streamAndLogLayout->addLayout(logLayout, 1);

    // ✅ 카메라 등록 / 콤보박스
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

    // ✅ 하단도 좌우 2:1 레이아웃으로 배치
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addLayout(leftControlLayout, 2);
    bottomLayout->addLayout(rightControlLayout, 1);

    // ✅ 최종 전체 구성
    QVBoxLayout *outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->addLayout(topLayout);
    outerLayout->addLayout(labelLayout);
    outerLayout->addLayout(streamAndLogLayout);
    outerLayout->addLayout(bottomLayout);
}
*/

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    setMinimumSize(1100, 700);  // ✅ 최소 크기 지정

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
    QHBoxLayout *streamAndLogLayout = new QHBoxLayout();
    streamAndLogLayout->addWidget(videoWidget);
    streamAndLogLayout->addLayout(logLayout);
    streamAndLogLayout->setStretch(0, 2);  // 비디오 2
    streamAndLogLayout->setStretch(1, 1);  // 로그 1

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

    // ✅ 하단 제어 영역 수평 배치 (2:1 비율)
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addLayout(leftControlLayout);
    bottomLayout->addLayout(rightControlLayout);
    bottomLayout->setStretch(0, 2);  // 버튼 쪽
    bottomLayout->setStretch(1, 1);  // 체크박스 쪽

    // ✅ 최종 전체 구성
    QVBoxLayout *outerLayout = new QVBoxLayout(centralWidget);
    outerLayout->addLayout(topLayout);
    outerLayout->addLayout(labelLayout);
    outerLayout->addLayout(streamAndLogLayout);
    outerLayout->addSpacing(10);  // ✅ 스트리밍 영역과 하단 제어 영역 사이 간격
    outerLayout->addLayout(bottomLayout);
}

void MainWindow::setupVideoStreamingSection()
{
    videoSection = new QWidget();
    QVBoxLayout *videoLayout = new QVBoxLayout(videoSection);
    videoLayout->setSpacing(5);
    videoLayout->setContentsMargins(0, 0, 0, 0);

    videoStreamingLabel = new QLabel("Video Streaming");
    videoStreamingLabel->setStyleSheet("color: #ff8c00; font-size: 14px; font-weight: bold;");
    videoLayout->addWidget(videoStreamingLabel);

    videoWidget = new QVideoWidget();
    videoWidget->setMinimumSize(400, 300);
    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);
    mediaPlayer->setSource(QUrl("rtsp://192.168.0.66:8554/mystream"));
    mediaPlayer->play();

    videoLayout->addWidget(videoWidget);

    mainLayout->addWidget(videoSection, 2);
}

void MainWindow::setupLogSection()
{
    logSection = new QWidget();
    logSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *logLayout = new QVBoxLayout(logSection);
    logLayout->setContentsMargins(0, 0, 0, 0);
    logLayout->setSpacing(5);

    logLabel = new QLabel("Safety Alerts");
    logLabel->setStyleSheet("color: #ff8c00; font-size: 14px; font-weight: bold;");

    logHistoryButton = new QPushButton("전체 로그 보기");
    logHistoryButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    logHistoryButton->setMinimumHeight(28);

    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    logHeaderLayout->addWidget(logLabel);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(logHistoryButton);

    logTable = new QTableWidget(0, 2);
    logTable->setHorizontalHeaderLabels(QStringList() << "Camera" << "Alert");
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->verticalHeader()->setVisible(false);
    logTable->setAlternatingRowColors(true);
    logTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    logTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    logLayout->addLayout(logHeaderLayout);
    logLayout->addWidget(logTable);
    logLayout->addWidget(logHistoryButton);

    mainLayout->addWidget(logSection, 1);

    connect(logHistoryButton, &QPushButton::clicked, this, &MainWindow::onLogHistoryClicked);
}

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

        // ✅ (선택) IP 주소를 기반으로 RTSP 주소 바꿔치기
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
        ppeDetectorCheckBox->setChecked(false);  // PPE Detector 끄기
    }

    QString status = enabled ? "enabled" : "disabled";
    addLogEntry("System", QString("Mosaicer %1").arg(status));
}

void MainWindow::onPPEDetectorToggled(bool enabled)
{
    if (enabled && mosaicerCheckBox->isChecked()) {
        mosaicerCheckBox->setChecked(false);  // Mosaicer 끄기
    }

    QString status = enabled ? "enabled" : "disabled";
    addLogEntry("System", QString("PPE Detector %1").arg(status));
}

