#include "mainwindow.h"
#include "cameralistdialog.h"
#include "loghistorydialog.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setWindowTitle("Smart SafetyNet");
    setMinimumSize(1000, 700);

    networkManager = new QNetworkAccessManager(this);

    setStyleSheet(R"(
        QWidget { background-color: #2b2b2b; color: white; }
        QLabel { color: white; }
        QTableWidget { background-color: #404040; color: white; gridline-color: #555; }
        QHeaderView::section { background-color: #353535; color: white; font-weight: bold; }
        QPushButton {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 6px;
            border-radius: 4px;
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

    QLabel *greetingLabel = new QLabel("Hello admin!");
    greetingLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

    QPushButton *exitButton = new QPushButton("종료");
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(greetingLabel);
    topLayout->addStretch();
    topLayout->addWidget(exitButton);

    QLabel *streamingLabel = new QLabel("Video Streaming");
    streamingLabel->setStyleSheet("font-weight: bold; color: orange;");

    cameraListButton = new QPushButton("카메라 리스트");
    connect(cameraListButton, &QPushButton::clicked, this, &MainWindow::onCameraListClicked);

    QHBoxLayout *streamingHeaderLayout = new QHBoxLayout();
    streamingHeaderLayout->addWidget(streamingLabel);
    streamingHeaderLayout->addStretch();
    streamingHeaderLayout->addWidget(cameraListButton);

    videoArea = new QWidget();
    videoGridLayout = new QGridLayout(videoArea);
    videoGridLayout->setContentsMargins(0, 0, 0, 0);
    videoGridLayout->setSpacing(1);
    videoGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(videoArea);
    scrollArea->setFixedWidth(2 * 320 + 3);

    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addLayout(streamingHeaderLayout);
    videoLayout->addWidget(scrollArea);

    QWidget *videoSection = new QWidget();
    videoSection->setLayout(videoLayout);
    videoSection->setFixedWidth(640);
    videoSection->setStyleSheet("border: 1px solid red;");

    QLabel *alertLabel = new QLabel("Alert");
    alertLabel->setStyleSheet("font-weight: bold; color: orange;");

    QPushButton *logHistoryButton = new QPushButton("전체 로그 보기");
    connect(logHistoryButton, &QPushButton::clicked, this, &MainWindow::onLogHistoryClicked);

    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    logHeaderLayout->addWidget(alertLabel);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(logHistoryButton);

    logTable = new QTableWidget();
    logTable->setColumnCount(2);
    logTable->setHorizontalHeaderLabels({"Camera", "Alert"});
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->verticalHeader()->setVisible(false);

    QVBoxLayout *logLayout = new QVBoxLayout();
    logLayout->addLayout(logHeaderLayout);
    logLayout->addWidget(logTable);

    QWidget *logSection = new QWidget();
    logSection->setLayout(logLayout);
    logSection->setStyleSheet("border: 1px solid red;");
    logSection->setMinimumWidth(320);

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

    rawCheckBox = new QCheckBox("Raw");
    blurCheckBox = new QCheckBox("Blur");
    ppeDetectorCheckBox = new QCheckBox("PPE Detector");

    connect(rawCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            blurCheckBox->setChecked(false);
            ppeDetectorCheckBox->setChecked(false);
            switchStreamForAllPlayers("raw");  // ✅ 여기서 raw 스트림 전환
            addLogEntry("System", "Raw mode enabled");
        }
    });

    connect(blurCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->setChecked(false);
            ppeDetectorCheckBox->setChecked(false);
            if (!cameraList.isEmpty()) {
                sendModeChangeRequest("blur", cameraList.first());
            }
            switchStreamForAllPlayers("processed");  // ✅ processed 전환
            addLogEntry("System", "Blur mode enabled");
        } else {
            if (!rawCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked())
                rawCheckBox->setChecked(true);
        }
    });

    connect(ppeDetectorCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->setChecked(false);
            blurCheckBox->setChecked(false);
            if (!cameraList.isEmpty()) {
                sendModeChangeRequest("detect", cameraList.first());
            }
            switchStreamForAllPlayers("processed");  // ✅ processed 전환
        } else {
            if (!rawCheckBox->isChecked() && !blurCheckBox->isChecked())
                rawCheckBox->setChecked(true);
        }
        addLogEntry("System", QString("PPE Detector %1").arg(checked ? "enabled" : "disabled"));
    });

    QVBoxLayout *functionLayout = new QVBoxLayout();
    functionLayout->addWidget(functionLabelButton);
    functionLayout->addWidget(rawCheckBox);
    functionLayout->addWidget(blurCheckBox);
    functionLayout->addWidget(ppeDetectorCheckBox);
    functionLayout->addStretch();

    QWidget *functionSection = new QWidget();
    functionSection->setLayout(functionLayout);
    functionSection->setFixedWidth(200);
    functionSection->setStyleSheet("border: 1px solid red;");

    QHBoxLayout *mainBodyLayout = new QHBoxLayout();
    mainBodyLayout->addWidget(videoSection);
    mainBodyLayout->addWidget(logSection);
    mainBodyLayout->addWidget(functionSection);

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

    cameraListDialog->refreshTable();
    cameraListDialog->show();
    cameraListDialog->raise();
    cameraListDialog->activateWindow();
}

void MainWindow::refreshVideoGrid()
{
    QLayoutItem *child;
    while ((child = videoGridLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
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

    // ✅ 카메라가 있고 Blur/PPE가 선택 안되어 있을 때만 Raw 체크
    if (!cameraList.isEmpty() && !blurCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()) {
        rawCheckBox->setChecked(true);
    }
    if (cameraList.isEmpty()) {
        rawCheckBox->blockSignals(true);
        blurCheckBox->blockSignals(true);
        ppeDetectorCheckBox->blockSignals(true);

        rawCheckBox->setChecked(false);
        blurCheckBox->setChecked(false);
        ppeDetectorCheckBox->setChecked(false);

        rawCheckBox->blockSignals(false);
        blurCheckBox->blockSignals(false);
        ppeDetectorCheckBox->blockSignals(false);        // ✅ 모든 카메라 삭제 시 모든 기능 체크 해제
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

void MainWindow::sendModeChangeRequest(const QString &mode, const CameraInfo &camera)
{
    if (camera.ip.isEmpty() || camera.port.isEmpty()) {
        QMessageBox::warning(this, "카메라 정보 오류", "카메라 IP 또는 포트가 비어있습니다.");
        return;
    }

    QString apiUrl = QString("http://%1/api/mode").arg(camera.ip);
    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["mode"] = mode;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

            QString status = responseDoc["status"].toString();
            QString message = responseDoc["message"].toString();

            if (status != "success") {
                QMessageBox::warning(this, "모드 변경 실패", message);
            }
        } else {
            QMessageBox::critical(this, "네트워크 오류", reply->errorString());
        }
    });
}



void MainWindow::switchStreamForAllPlayers(const QString &suffix)
{
    for (int i = 0; i < cameraList.size() && i < players.size(); ++i) {
        QString streamUrl = QString("rtsps://%1:%2/%3")
        .arg(cameraList[i].ip)
            .arg(cameraList[i].port)
            .arg(suffix);

        players[i]->stop();  // 기존 스트림 중지
        players[i]->setSource(QUrl(streamUrl));
        players[i]->play();  // 새 스트림 시작
    }
}
