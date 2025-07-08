#include "mainwindow.h"
#include "cameralistdialog.h"
#include "loghistorydialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QTableWidget>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentCameraNumber(1)
{
    setupUI();
    setWindowTitle("Smart SafetyNet");
    setMinimumSize(1000, 700);

    setStyleSheet(R"(
        QWidget { background-color: #2b2b2b; color: white; }
        QLabel { color: white; }
        QTableWidget { background-color: #404040; color: white; gridline-color: #555; }
        QHeaderView::section { background-color: #353535; color: white; font-weight: bold; }
        QPushButton {
            background-color: #404040; color: white;
            border: 1px solid #555; padding: 6px; border-radius: 4px;
        }
        QPushButton:hover { background-color: #505050; }
        QCheckBox { color: white; }
    )");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ✅ 최상단 인사말 + 종료 버튼
    QLabel *greetingLabel = new QLabel("Hello admin!");
    greetingLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

    QPushButton *exitButton = new QPushButton("종료");
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(greetingLabel);
    topLayout->addStretch();
    topLayout->addWidget(exitButton);

    // ✅ Video Streaming 상단 라벨 + 버튼
    QLabel *streamingLabel = new QLabel("Video Streaming");
    streamingLabel->setStyleSheet("font-weight: bold; color: orange;");

    cameraListButton = new QPushButton("카메라 리스트");
    connect(cameraListButton, &QPushButton::clicked, this, &MainWindow::onCameraListClicked);

    QHBoxLayout *streamingHeaderLayout = new QHBoxLayout();
    streamingHeaderLayout->addWidget(streamingLabel);
    streamingHeaderLayout->addStretch();
    streamingHeaderLayout->addWidget(cameraListButton);

    // ✅ Video Streaming Area
    videoArea = new QWidget();
    videoGridLayout = new QGridLayout(videoArea);
    videoGridLayout->setContentsMargins(0, 0, 0, 0);
    videoGridLayout->setHorizontalSpacing(1);
    videoGridLayout->setVerticalSpacing(1);
    videoGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(videoArea);
    scrollArea->setFixedWidth(2 * 320 + 3);
    scrollArea->setMinimumHeight(0);  // ✅ 하단 빈공간 줄이기
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addLayout(streamingHeaderLayout);
    videoLayout->addWidget(scrollArea);

    QWidget *videoSection = new QWidget();
    videoSection->setLayout(videoLayout);
    videoSection->setFixedWidth(640);
    videoSection->setStyleSheet("border: 1px solid red;");

    // ✅ Log 상단 라벨 + 버튼
    QLabel *alertLabel = new QLabel("Alert");
    alertLabel->setStyleSheet("font-weight: bold; color: orange;");

    QPushButton *logHistoryButton = new QPushButton("전체 로그 보기");
    connect(logHistoryButton, &QPushButton::clicked, this, &MainWindow::onLogHistoryClicked);

    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    logHeaderLayout->addWidget(alertLabel);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(logHistoryButton);

    // ✅ Log Table
    logTable = new QTableWidget();
    logTable->setColumnCount(2);
    logTable->setHorizontalHeaderLabels({"Camera", "Alert"});
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    logTable->verticalHeader()->setVisible(false);

    QVBoxLayout *logLayout = new QVBoxLayout();
    logLayout->addLayout(logHeaderLayout);
    logLayout->addWidget(logTable);

    QWidget *logSection = new QWidget();
    logSection->setLayout(logLayout);
    logSection->setStyleSheet("border: 1px solid red;");
    logSection->setMinimumWidth(320);  // ✅ 스크롤 방지용 너비 확보

    // ✅ Function 상단 텍스트 (버튼 기반) + 스타일 개선
    QPushButton *functionLabelButton = new QPushButton("Function");
    functionLabelButton->setFlat(true);
    functionLabelButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: orange;
            font-weight: bold;
            border: 1px solid red;
        }
        QPushButton:hover {
            color: #ffae42;
        }
    )");

    ppeDetectorCheckBox = new QCheckBox("PPE Detector");
    connect(ppeDetectorCheckBox, &QCheckBox::toggled, this, &MainWindow::onPPEDetectorToggled);

    mosaicerCheckBox = new QCheckBox("Mosaicer");
    connect(mosaicerCheckBox, &QCheckBox::toggled, this, &MainWindow::onMosaicerToggled);

    QVBoxLayout *functionLayout = new QVBoxLayout();
    functionLayout->addWidget(functionLabelButton);
    functionLayout->addWidget(ppeDetectorCheckBox);
    functionLayout->addWidget(mosaicerCheckBox);
    functionLayout->addStretch();

    QWidget *functionSection = new QWidget();
    functionSection->setLayout(functionLayout);
    functionSection->setFixedWidth(200);
    functionSection->setStyleSheet("border: 1px solid red;");

    // ✅ 영상 + 로그 + 기능 수평 정렬
    QHBoxLayout *mainBodyLayout = new QHBoxLayout();
    mainBodyLayout->addWidget(videoSection);
    mainBodyLayout->addWidget(logSection);
    mainBodyLayout->addWidget(functionSection);

    // ✅ 전체 수직 레이아웃
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(mainBodyLayout);

    refreshVideoGrid();
}

void MainWindow::onCameraListClicked()
{
    if (!cameraListDialog) {
        cameraListDialog = new CameraListDialog(this, &cameraList);
        connect(cameraListDialog, &CameraListDialog::cameraListUpdated, this, &MainWindow::refreshVideoGrid);
    }

    cameraListDialog->refreshTable();  // ✅ 항상 최신 정보로 갱신
    cameraListDialog->show();
    cameraListDialog->raise();
    cameraListDialog->activateWindow();
}


void MainWindow::refreshVideoGrid()
{
    QLayoutItem *child;
    while ((child = videoGridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    for (QMediaPlayer *player : players) {
        player->stop();
        delete player;
    }
    players.clear();
    videoWidgets.clear();

    int total = std::max(4, static_cast<int>(cameraList.size()));
    int columns = 2;
    int rows = (total + 1) / 2;

    // ✅ videoArea 크기 강제 지정 (스크롤바 방지)
    videoArea->setMinimumSize(columns * 320, rows * 240);

    for (int i = 0; i < total; ++i) {
        QWidget *videoFrame = new QWidget();
        videoFrame->setFixedSize(320, 240);
        videoFrame->setStyleSheet("background-color: black; border: 1px solid #555;");

        if (i < cameraList.size()) {
            QLabel *nameLabel = new QLabel(cameraList[i].name, videoFrame);
            nameLabel->setStyleSheet("color: white; font-weight: bold; background-color: rgba(0,0,0,100); padding: 2px;");
            nameLabel->move(5, 5);
            nameLabel->show();

            QVideoWidget *vw = new QVideoWidget(videoFrame);
            vw->setGeometry(0, 0, 320, 240);
            vw->lower();

            QMediaPlayer *player = new QMediaPlayer(this);
            player->setVideoOutput(vw);
            player->setSource(QUrl(cameraList[i].rtspUrl()));
            player->play();

            players.append(player);
            videoWidgets.append(vw);
        } else {
            QLabel *noCam = new QLabel("No Camera", videoFrame);
            noCam->setAlignment(Qt::AlignCenter);
            noCam->setGeometry(0, 0, 320, 240);
            noCam->setStyleSheet("color: white;");
        }

        videoGridLayout->addWidget(videoFrame, i / columns, i % columns);
    }
}


void MainWindow::addLogEntry(const QString &camera, const QString &alert)
{
    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(camera));
    logTable->setItem(0, 1, new QTableWidgetItem(alert));

    if (logTable->rowCount() > 20)
        logTable->removeRow(logTable->rowCount() - 1);
}

void MainWindow::onLogHistoryClicked()
{
    LogHistoryDialog dialog(this);
    dialog.exec();
}

void MainWindow::onPPEDetectorToggled(bool enabled)
{
    if (enabled && mosaicerCheckBox->isChecked())
        mosaicerCheckBox->setChecked(false);
    addLogEntry("System", QString("PPE Detector %1").arg(enabled ? "enabled" : "disabled"));
}

void MainWindow::onMosaicerToggled(bool enabled)
{
    if (enabled && ppeDetectorCheckBox->isChecked())
        ppeDetectorCheckBox->setChecked(false);
    addLogEntry("System", QString("Mosaicer %1").arg(enabled ? "enabled" : "disabled"));
}
