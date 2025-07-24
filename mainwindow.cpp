#include "mainwindow.h"
#include "cameralistdialog.h"
#include "loghistorydialog.h"

// UI 관련 위젯
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

// 알림창
#include <QMessageBox>

#include <QHeaderView>

// JSON 데이터 파싱용
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// 네트워크 요청 처리
#include <QNetworkReply>

// 주기적인 작업용
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    videoPlayerManager = new VideoPlayerManager(this);

    setupUI();
    setWindowTitle("Smart SafetyNet");
    showMaximized();  // ✅ 전체 화면으로 시작

    // REST API 통신용
    networkManager = new QNetworkAccessManager(this);

    // mainwindow의 스타일 시트 설정 : 전체 윈도우 스타일에 적용 - 다크모드, 버튼/테이블/라벨 전체 통일 디자인
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

void MainWindow::setupUI() {
    centralWidget = new QWidget(this);
    centralWidget->setContentsMargins(0, 0, 0, 0);
    // centralWidget->setStyleSheet("border: 3px solid blue;");


    setCentralWidget(centralWidget);

    setupTopBar();
    setupOnvifSection();
    setupVideoSection();
    setupLogSection();
    setupFunctionPanel();
    setupMainLayout();

    refreshVideoGrid();
}

void MainWindow::setupTopBar() {
    QLabel *greetingLabel = new QLabel("Hello admin!");
    greetingLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

    QPushButton *exitButton = new QPushButton("종료");
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::close);

    topLayout = new QHBoxLayout();
    topLayout->addWidget(greetingLabel);
    topLayout->addStretch();
    topLayout->addWidget(exitButton);
}

void MainWindow::setupOnvifSection()
{
    onvifPlayer = new QMediaPlayer(this);
    onvifVideoItem = new QGraphicsVideoItem();
    onvifVideoItem->setSize(QSizeF(640, 360));

    onvifScene = new QGraphicsScene(this);
    onvifScene->addItem(onvifVideoItem);

    onvifView = new QGraphicsView(onvifScene);
    onvifView->setFixedSize(640, 360);
    onvifView->setStyleSheet("background-color: black; border: none; margin: 0px; padding: 0px;");

    onvifPlayer->setVideoOutput(onvifVideoItem);
    onvifPlayer->setSource(QUrl("rtsp://192.168.0.35:554/0/onvif/profile2/media.smp"));
    onvifPlayer->play();

    onvifSection = new QWidget();
    onvifSection->setFixedHeight(400);  // 360 + label 여유
    QVBoxLayout *onvifLayout = new QVBoxLayout(onvifSection);  // ✅ 이름 변경
    onvifLayout->setContentsMargins(10, 10, 10, 10);
    onvifLayout->setSpacing(10);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    QLabel *label = new QLabel("ONVIF Camera Stream");
    label->setStyleSheet("font-weight: bold; color: orange;");
    labelLayout->addWidget(label);
    labelLayout->addStretch();  // 선택사항: 오른쪽 정렬용
    qDebug() << "ONVIF height:" << label->sizeHint().height();

    onvifLayout->addLayout(labelLayout);
    onvifLayout->addWidget(onvifView);

    // onvifSection->setStyleSheet("border: 2px solid red;");

}

void MainWindow::setupPiVideoSection()
{
    QLabel *streamingLabel = new QLabel("Video Streaming");
    streamingLabel->setStyleSheet("font-weight: bold; color: orange;");

    cameraListButton = new QPushButton("카메라 리스트");
    connect(cameraListButton, &QPushButton::clicked, this, &MainWindow::onCameraListClicked);

    streamingHeaderLayout = new QHBoxLayout();
    streamingHeaderLayout->setContentsMargins(0, 0, 0, 0);  // ✅ 좌우 여백 제거
    streamingHeaderLayout->setSpacing(5);                  // ✅ 라벨-버튼 간격 줄임
    streamingHeaderLayout->addWidget(streamingLabel);
    streamingHeaderLayout->addStretch();
    streamingHeaderLayout->addWidget(cameraListButton);

    videoArea = new QWidget();
    videoGridLayout = new QGridLayout(videoArea);
    videoGridLayout->setContentsMargins(0, 0, 0, 0);         // ✅ 내부 여백 제거
    videoGridLayout->setSpacing(3);                          // 영상 간격만 유지
    videoGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // videoArea->setStyleSheet("border: 2px dotted green;");

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(videoArea);
    scrollArea->setFixedWidth(2 * 320 + 20);  // scroll bar 고려 여유 포함
    scrollArea->setFrameStyle(QFrame::NoFrame);
}


// 전체 VideoSection 조립 (ONVIF + Pi 영상)
void MainWindow::setupVideoSection()
{
    setupPiVideoSection();  // ✅ 내부적으로 Pi 영상 구성 먼저 수행

    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addLayout(streamingHeaderLayout);  // 상단: "Video Streaming", 카메라 리스트 버튼
    videoLayout->addWidget(onvifFrame);             // 중단: ONVIF 영상
    videoLayout->addWidget(scrollArea);             // 하단: Pi 영상들

    videoSection = new QWidget();
    videoSection->setLayout(videoLayout);
    videoSection->setFixedWidth(640 + 20);
    // videoSection->setStyleSheet("border: 2px solid red;");

}

void MainWindow::setupLogSection() {
    QLabel *alertLabel = new QLabel("Alert");
    alertLabel->setStyleSheet("font-weight: bold; color: orange;");
    qDebug() << "Alert height:" << alertLabel->sizeHint().height();

    QPushButton *logHistoryButton = new QPushButton("전체 로그 보기");
    connect(logHistoryButton, &QPushButton::clicked, this, &MainWindow::onLogHistoryClicked);

    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    logHeaderLayout->addWidget(alertLabel);
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(logHistoryButton);

    logTable = new QTableWidget();
    logTable->setColumnCount(5);
    logTable->setHorizontalHeaderLabels({"Camera Name", "Date", "Time", "Function", "Event"});
    logTable->horizontalHeader()->setStretchLastSection(true);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->verticalHeader()->setVisible(false);

    connect(logTable, &QTableWidget::cellClicked, this, &MainWindow::onAlertItemClicked);

    QVBoxLayout *logLayout = new QVBoxLayout();
    logLayout->addLayout(logHeaderLayout);
    logLayout->addWidget(logTable);

    logSection = new QWidget();
    logSection->setLayout(logLayout);
    logSection->setMinimumWidth(320);
    // logSection->setStyleSheet("border: 2px solid red;");

}

void MainWindow::setupFunctionPanel() {
    QPushButton *functionLabelButton = new QPushButton("Function");
    functionLabelButton->setFlat(true);
    functionLabelButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: orange;
            font-weight: bold;
        }
        QPushButton:hover {
            color: #ffae42;
        }
    )");

    rawCheckBox = new QCheckBox("Raw");
    blurCheckBox = new QCheckBox("Blur");
    ppeDetectorCheckBox = new QCheckBox("PPE Detector");
    nightIntrusionCheckBox = new QCheckBox("Night Intrusion");
    fallDetectionCheckBox = new QCheckBox("Fall Detection");

    // ✅ Raw 체크박스
    connect(rawCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            blurCheckBox->blockSignals(true); blurCheckBox->setChecked(false); blurCheckBox->blockSignals(false);
            ppeDetectorCheckBox->blockSignals(true); ppeDetectorCheckBox->setChecked(false); ppeDetectorCheckBox->blockSignals(false);
            nightIntrusionCheckBox->blockSignals(true); nightIntrusionCheckBox->setChecked(false); nightIntrusionCheckBox->blockSignals(false);
            fallDetectionCheckBox->blockSignals(true); fallDetectionCheckBox->setChecked(false); fallDetectionCheckBox->blockSignals(false);

            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("raw", camera);
            switchStreamForAllPlayers("processed");
            addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
        }
    });

    // ✅ Blur 체크박스
    connect(blurCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true); rawCheckBox->setChecked(false); rawCheckBox->blockSignals(false);
            ppeDetectorCheckBox->blockSignals(true); ppeDetectorCheckBox->setChecked(false); ppeDetectorCheckBox->blockSignals(false);
            nightIntrusionCheckBox->blockSignals(true); nightIntrusionCheckBox->setChecked(false); nightIntrusionCheckBox->blockSignals(false);
            fallDetectionCheckBox->blockSignals(true); fallDetectionCheckBox->setChecked(false); fallDetectionCheckBox->blockSignals(false);

            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("blur", camera);
            switchStreamForAllPlayers("processed");
            addLogEntry("System", "Blur", "Blur mode enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()
                && !nightIntrusionCheckBox->isChecked() && !fallDetectionCheckBox->isChecked()) {
                rawCheckBox->setChecked(true);
            }
        }
    });

    // ✅ PPE Detector 체크박스
    connect(ppeDetectorCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true); rawCheckBox->setChecked(false); rawCheckBox->blockSignals(false);
            blurCheckBox->blockSignals(true); blurCheckBox->setChecked(false); blurCheckBox->blockSignals(false);
            nightIntrusionCheckBox->blockSignals(true); nightIntrusionCheckBox->setChecked(false); nightIntrusionCheckBox->blockSignals(false);
            fallDetectionCheckBox->blockSignals(true); fallDetectionCheckBox->setChecked(false); fallDetectionCheckBox->blockSignals(false);

            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("detect", camera);
            switchStreamForAllPlayers("processed");
            addLogEntry("System", "PPE", "PPE Detector enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !blurCheckBox->isChecked()
                && !nightIntrusionCheckBox->isChecked() && !fallDetectionCheckBox->isChecked()) {
                rawCheckBox->setChecked(true);
            }
        }
    });

    // ✅ Night Intrusion 체크박스
    connect(nightIntrusionCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true); rawCheckBox->setChecked(false); rawCheckBox->blockSignals(false);
            blurCheckBox->blockSignals(true); blurCheckBox->setChecked(false); blurCheckBox->blockSignals(false);
            ppeDetectorCheckBox->blockSignals(true); ppeDetectorCheckBox->setChecked(false); ppeDetectorCheckBox->blockSignals(false);
            fallDetectionCheckBox->blockSignals(true); fallDetectionCheckBox->setChecked(false); fallDetectionCheckBox->blockSignals(false);

            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("trespass", camera);
            switchStreamForAllPlayers("processed");
            addLogEntry("System", "Night", "Night Intrusion enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !blurCheckBox->isChecked()
                && !ppeDetectorCheckBox->isChecked() && !fallDetectionCheckBox->isChecked()) {
                rawCheckBox->setChecked(true);
            }
        }
    });

    // ✅ Fall Detection 체크박스
    connect(fallDetectionCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true); rawCheckBox->setChecked(false); rawCheckBox->blockSignals(false);
            blurCheckBox->blockSignals(true); blurCheckBox->setChecked(false); blurCheckBox->blockSignals(false);
            ppeDetectorCheckBox->blockSignals(true); ppeDetectorCheckBox->setChecked(false); ppeDetectorCheckBox->blockSignals(false);
            nightIntrusionCheckBox->blockSignals(true); nightIntrusionCheckBox->setChecked(false); nightIntrusionCheckBox->blockSignals(false);

            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("fall", camera);
            switchStreamForAllPlayers("processed");
            addLogEntry("System", "Fall", "Fall Detection enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !blurCheckBox->isChecked()
                && !ppeDetectorCheckBox->isChecked() && !nightIntrusionCheckBox->isChecked()) {
                rawCheckBox->setChecked(true);
            }
        }
    });

    QPushButton *healthCheckButton = new QPushButton("헬시 체크");
    connect(healthCheckButton, &QPushButton::clicked, this, &MainWindow::performHealthCheck);

    QVBoxLayout *functionLayout = new QVBoxLayout();
    functionLayout->addWidget(functionLabelButton);
    functionLayout->addWidget(rawCheckBox);
    functionLayout->addWidget(blurCheckBox);
    functionLayout->addWidget(ppeDetectorCheckBox);
    functionLayout->addWidget(nightIntrusionCheckBox);
    functionLayout->addWidget(fallDetectionCheckBox);
    functionLayout->addStretch();

    functionLayout->addWidget(healthCheckButton);

    functionSection = new QWidget();
    functionSection->setLayout(functionLayout);
    functionSection->setFixedWidth(200);
    // functionSection->setStyleSheet("border: 2px solid red;");

}

void MainWindow::setupMainLayout() {
    QVBoxLayout *mainWindowLayout = new QVBoxLayout(centralWidget);
    mainWindowLayout->setContentsMargins(0, 10, 0, 0);
    mainWindowLayout->setSpacing(10);  // Top ↔ Body 간격만 유지

    // Top Bar (Hello admin! + 종료 버튼)
    mainWindowLayout->addLayout(topLayout);

    // 좌측 열: ONVIF + Pi 영상
    QVBoxLayout *leftColumnLayout = new QVBoxLayout();
    leftColumnLayout->setSpacing(0);
    leftColumnLayout->setContentsMargins(0, 0, 0, 0);
    leftColumnLayout->addWidget(onvifSection);
    leftColumnLayout->addWidget(videoSection);
    leftColumnLayout->setAlignment(Qt::AlignTop);

    QWidget *leftColumnWidget = new QWidget();
    leftColumnWidget->setLayout(leftColumnLayout);
    leftColumnWidget->setFixedWidth(640 + 20);

    // 중앙 열: 로그
    QVBoxLayout *middleColumnLayout = new QVBoxLayout();
    middleColumnLayout->setContentsMargins(0, 0, 0, 0);
    middleColumnLayout->setSpacing(0);
    middleColumnLayout->addWidget(logSection);

    QWidget *middleColumnWidget = new QWidget();
    middleColumnWidget->setLayout(middleColumnLayout);

    // 우측 열: 기능 패널
    QVBoxLayout *rightColumnLayout = new QVBoxLayout();
    rightColumnLayout->setContentsMargins(0, 0, 0, 0);
    rightColumnLayout->setSpacing(0);
    rightColumnLayout->addWidget(functionSection);

    QWidget *rightColumnWidget = new QWidget();
    rightColumnWidget->setLayout(rightColumnLayout);
    rightColumnWidget->setFixedWidth(200);

    // 본문 3열
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setAlignment(Qt::AlignTop);
    bodyLayout->addWidget(leftColumnWidget);
    bodyLayout->addWidget(middleColumnWidget);
    bodyLayout->addWidget(rightColumnWidget);

    // leftColumnWidget->setStyleSheet("border: 2px dashed orange;");
    // middleColumnWidget->setStyleSheet("border: 2px dashed orange;");
    // rightColumnWidget->setStyleSheet("border: 2px dashed orange;");


    // 전체 적용
    mainWindowLayout->addLayout(bodyLayout);
}

void MainWindow::refreshVideoGrid()
{
    if (!videoGridLayout || !videoArea || !videoPlayerManager) {
        qWarning() << "[refreshVideoGrid] 필수 구성요소가 아직 초기화되지 않았습니다.";
        return;
    }

    // 화면 크기 조정
    int total = std::max(4, static_cast<int>(cameraList.size()));
    int columns = 2;
    int rows = (total + 1) / 2;
    videoArea->setMinimumSize(columns * 320, rows * 240);

    // ✅ 스트림 suffix는 항상 processed 고정
    QString streamSuffix = "processed";

    // ✅ 아무 모드도 체크되지 않은 경우 → raw 모드 적용 및 서버에 먼저 전송
    bool isRawMode = false;
    if (!cameraList.isEmpty()
        && !rawCheckBox->isChecked()
        && !blurCheckBox->isChecked()
        && !ppeDetectorCheckBox->isChecked()
        && !nightIntrusionCheckBox->isChecked()
        && !fallDetectionCheckBox->isChecked()) {

        rawCheckBox->blockSignals(true);
        rawCheckBox->setChecked(true);
        rawCheckBox->blockSignals(false);

        for (const CameraInfo &camera : cameraList)
            sendModeChangeRequest("raw", camera);  // ✅ 먼저 서버에 raw 모드 요청

        isRawMode = true;
    }

    // ✅ 스트리밍 구성: 항상 processed 스트림 사용
    videoPlayerManager->setupVideoGrid(videoGridLayout, cameraList, streamSuffix);

    // ✅ 카메라 리스트가 비어 있으면 체크박스 초기화
    if (cameraList.isEmpty()) {
        rawCheckBox->blockSignals(true);
        blurCheckBox->blockSignals(true);
        ppeDetectorCheckBox->blockSignals(true);
        nightIntrusionCheckBox->blockSignals(true);
        fallDetectionCheckBox->blockSignals(true);

        rawCheckBox->setChecked(false);
        blurCheckBox->setChecked(false);
        ppeDetectorCheckBox->setChecked(false);
        nightIntrusionCheckBox->setChecked(false);
        fallDetectionCheckBox->setChecked(false);

        rawCheckBox->blockSignals(false);
        blurCheckBox->blockSignals(false);
        ppeDetectorCheckBox->blockSignals(false);
        nightIntrusionCheckBox->blockSignals(false);
        fallDetectionCheckBox->blockSignals(false);
    }

    // ✅ 로그 출력은 실제 적용 이후로
    if (isRawMode) {
        addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
    }

    setupWebSocketConnections();
    loadInitialLogs();       // 초기 로그 불러오기
    performHealthCheck();    // 상태 체크

    if (onvifFrame) {
        onvifFrame->show();
        onvifFrame->raise();
    }

    if (onvifPlayer && onvifPlayer->playbackState() != QMediaPlayer::PlayingState) {
        onvifPlayer->play();
        qDebug() << "[ONVIF] 강제 재생 재요청됨";
    }

    QTimer::singleShot(200, this, [this]() {
        if (onvifFrame) {
            onvifFrame->show();
            onvifFrame->raise();  // 혹시 다른 것에 가려졌다면
        }

        onvifPlayer = new QMediaPlayer(this);
        onvifPlayer->setVideoOutput(onvifVideo);  // 기존 video 위젯 유지
        onvifPlayer->setSource(QUrl("rtsp://192.168.0.35:554/0/onvif/profile2/media.smp"));
        onvifPlayer->play();

        qDebug() << "[ONVIF] 완전 새로 생성 후 재생됨";
    });
}

void MainWindow::switchStreamForAllPlayers(const QString &suffix)
{
    if (videoPlayerManager)
        videoPlayerManager->switchStreamForAllPlayers(cameraList, suffix);
}

void MainWindow::addLogEntry(const QString &cameraName,
                             const QString &function,
                             const QString &event,
                             const QString &imagePath,
                             const QString &details,
                             const QString &ip)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QString time = QTime::currentTime().toString("HH:mm:ss");

    int zone = -1;
    for (int i = 0; i < cameraList.size(); ++i) {
        if (cameraList[i].name == cameraName) {
            zone = i + 1;
            break;
        }
    }

    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(cameraName));
    logTable->setItem(0, 1, new QTableWidgetItem(date));
    logTable->setItem(0, 2, new QTableWidgetItem(time));
    logTable->setItem(0, 3, new QTableWidgetItem(function));
    logTable->setItem(0, 4, new QTableWidgetItem(event));

    fullLogEntries.prepend({
        cameraName,
        function,
        event,
        imagePath,
        details,
        date,
        time,
        zone,
        ip
    });

    if (logTable->rowCount() > 20)
        logTable->removeRow(logTable->rowCount() - 1);
}


void MainWindow::onLogHistoryClicked()
{
    LogHistoryDialog dialog(this, &fullLogEntries);  // 로그 목록 전달
    dialog.exec();
}

void MainWindow::sendModeChangeRequest(const QString &mode, const CameraInfo &camera)
{
    if (camera.ip.isEmpty()) {
        qWarning() << "[모드 변경] 카메라 IP 없음 →" << camera.name;
        return;
    }

    if (!socketMap.contains(camera.ip)) {
        qWarning() << "[모드 변경] WebSocket 연결 없음 →" << camera.name;
        return;
    }

    QWebSocket *socket = socketMap[camera.ip];
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "[모드 변경] WebSocket 비연결 상태 →" << camera.name;
        return;
    }

    // ✅ WebSocket 메시지 생성
    QJsonObject payload;
    payload["type"] = "set_mode";
    payload["mode"] = mode;

    QJsonDocument doc(payload);
    QString message = doc.toJson(QJsonDocument::Compact);
    socket->sendTextMessage(message);

    qDebug() << "[WebSocket] 모드 변경 메시지 전송됨:" << message;

    // ✅ WebSocket 응답 메시지 처리 (이 socket만을 위한 임시 슬롯)
    connect(socket, &QWebSocket::textMessageReceived, this, [=](const QString &msg) {
        QJsonDocument respDoc = QJsonDocument::fromJson(msg.toUtf8());
        if (!respDoc.isObject()) return;

        QJsonObject obj = respDoc.object();
        QString type = obj["type"].toString();

        if (type == "mode_change_ack") {
            QString status = obj["status"].toString();
            QString serverMessage = obj["message"].toString();

            if (status == "error") {
                qWarning() << "[모드 변경 실패]" << serverMessage;
                QMessageBox::warning(this, "모드 변경 실패", serverMessage);
            } else {
                qDebug() << "[모드 변경 성공 응답]" << serverMessage;
            }
        }
    });
}

void MainWindow::onAlertItemClicked(int row, int column)
{
    if (row >= fullLogEntries.size()) return;

    const LogEntry &entry = fullLogEntries.at(row);
    if (entry.imagePath.isEmpty()) {
        QMessageBox::information(this, "이미지 없음", "이 항목에는 이미지가 없습니다.");
        return;
    }

    QString ip = entry.ip;
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "IP 없음", "카메라 IP가 없습니다.");
        return;
    }

    // 상대 경로일 경우 "./" 제거
    QString imagePath = entry.imagePath;
    if (imagePath.startsWith("../"))
        imagePath = imagePath.mid(3);

    QString urlStr = QString("http://%1/%2").arg(ip, imagePath);

    qDebug() << "[이미지 요청 URL]" << urlStr;

    QUrl url(urlStr);
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "이미지 로딩 실패", reply->errorString());
            return;
        }

        QPixmap pix;
        pix.loadFromData(reply->readAll());
        if (pix.isNull()) {
            QMessageBox::warning(this, "이미지 오류", "유효한 이미지가 아닙니다.");
            return;
        }

        QDialog *imgDialog = new QDialog(this);
        imgDialog->setWindowTitle("감지 이미지");
        QLabel *imgLabel = new QLabel();
        imgLabel->setPixmap(pix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QVBoxLayout *layout = new QVBoxLayout(imgDialog);
        layout->addWidget(imgLabel);
        imgDialog->setLayout(layout);
        imgDialog->setMinimumSize(640, 480);
        imgDialog->exec();
    });
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

void MainWindow::setupWebSocketConnections()
{
    for (const CameraInfo &camera : cameraList) {
        if (socketMap.contains(camera.ip)) continue;  // 이미 연결된 경우 생략

        QWebSocket *socket = new QWebSocket();

        connect(socket, &QWebSocket::sslErrors, this, [socket](const QList<QSslError> &) {
            socket->ignoreSslErrors();
        });

        connect(socket, &QWebSocket::connected, this, &MainWindow::onSocketConnected);
        connect(socket, &QWebSocket::disconnected, this, &MainWindow::onSocketDisconnected);
        /*connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                this, &MainWindow::onSocketErrorOccurred);*/
        connect(socket, &QWebSocket::errorOccurred,
                this, &MainWindow::onSocketErrorOccurred);
        connect(socket, &QWebSocket::textMessageReceived,
                this, &MainWindow::onSocketMessageReceived);

        QString wsUrl = QString("wss://%1:8443/ws").arg(camera.ip);
        socket->open(QUrl(wsUrl));
        socketMap[camera.ip] = socket;
    }
}

void MainWindow::onSocketMessageReceived(const QString &message)
{
    qDebug() << "[WebSocket 수신 메시지]" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "[WebSocket 메시지] JSON 파싱 실패";
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QJsonObject data = obj["data"].toObject();

    qDebug() << "📨 [WebSocket 타입]" << type;

    QString ipSender;
    QWebSocket *senderSocket = qobject_cast<QWebSocket*>(sender());
    for (auto it = socketMap.begin(); it != socketMap.end(); ++it) {
        if (it.value() == senderSocket) {
            ipSender = it.key();
            break;
        }
    }

    if (ipSender.isEmpty()) {
        qWarning() << "[WebSocket] 발신자 IP 찾기 실패";
        return;
    }

    const CameraInfo *cameraPtr = nullptr;
    for (int i = 0; i < cameraList.size(); ++i) {
        if (cameraList[i].ip.trimmed() == ipSender.trimmed()) {  // 공백 방지
            cameraPtr = &cameraList[i];
            break;
        }
    }

    if (!cameraPtr) {
        qWarning() << "[WebSocket] CameraInfo 찾기 실패 for IP:" << ipSender;
        return;
    }
    const CameraInfo &camera = *cameraPtr;

    if (type == "new_detection") {
        int person = data["person_count"].toInt();
        int helmet = data["helmet_count"].toInt();
        int vest = data["safety_vest_count"].toInt();
        double conf = data["avg_confidence"].toDouble();
        QString imagePath = data["image_path"].toString();
        QString ts = data["timestamp"].toString();

        QString event;
        QString details = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                              .arg(person).arg(helmet).arg(vest).arg(conf, 0, 'f', 2);

        // ✅ 기존 PPE 감지 처리
        if (helmet < person && vest >= person)
            event = "⛑️ 헬멧 미착용 감지";
        else if (vest < person && helmet >= person)
            event = "🦺 조끼 미착용 감지";
        else
            event = "⛑️ 🦺 PPE 미착용 감지";

        qDebug() << "[PPE 이벤트]" << event << "IP:" << camera.ip;

        // PPE 알람 연속 횟수 추적
        if (event.contains("미착용")) {
            int count = ppeViolationStreakMap[camera.name] + 1;
            ppeViolationStreakMap[camera.name] = count;

            if (count >= 4) {
                QMessageBox *popup = new QMessageBox(this);
                popup->setIcon(QMessageBox::Warning);
                popup->setWindowTitle("지속적인 PPE 위반");
                popup->setText(QString("%1 카메라에서 PPE 미착용이 연속 4회 감지되었습니다!").arg(camera.name));
                popup->setStandardButtons(QMessageBox::Ok);
                popup->setModal(false);         // ✅ 비모달 설정

                if (!imagePath.isEmpty()) {
                    QString cleanPath = imagePath;
                    if (cleanPath.startsWith("../"))
                        cleanPath = cleanPath.mid(3);  // 상대경로 정리

                    QString urlStr = QString("http://%1/%2").arg(camera.ip, cleanPath);
                    QUrl url(urlStr);
                    QNetworkRequest request(url);

                    QNetworkAccessManager *manager = new QNetworkAccessManager(popup);  // 팝업에 소속
                    QNetworkReply *reply = manager->get(request);

                    connect(reply, &QNetworkReply::finished, popup, [=]() {
                        reply->deleteLater();
                        QPixmap pix;
                        pix.loadFromData(reply->readAll());
                        if (!pix.isNull()) {
                            QLabel *imgLabel = new QLabel();
                            imgLabel->setPixmap(pix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                            popup->layout()->addWidget(imgLabel);
                            popup->adjustSize();  // 이미지 포함 크기 자동 조정
                        }
                    });
                }

                popup->show();                  // ✅ show()만 사용하여 non-blocking

                ppeViolationStreakMap[camera.name] = 0;  // 리셋
            }

        } else {
            ppeViolationStreakMap[camera.name] = 0;
        }

        addLogEntry(camera.name, "PPE", event, imagePath, details, camera.ip);
    }

    else if (type == "new_trespass") {
        QString ts = data["timestamp"].toString();
        int count = data["count"].toInt();

        if (count > 0) {
            QString event = QString("🌙 야간 침입 감지 (%1명)").arg(count);
            QString details = QString("감지 시각: %1 | 침입자 수: %2").arg(ts).arg(count);

            addLogEntry(camera.name, "Night", event, "", details, camera.ip);
        }
    }

    else if (type == "new_blur") {
        QString ts = data["timestamp"].toString();
        QString key = camera.name + "_" + ts;
        if (recentBlurLogKeys.contains(key)) {
            qDebug() << "[BLUR 중복 무시]" << key;
            return;
        }

        int count = data["count"].toInt();
        QString event = QString("🔍 %1명 감지").arg(count);

        qDebug() << "[Blur 이벤트]" << event << "IP:" << camera.ip;

        addLogEntry(camera.name, "Blur", event, "", "", camera.ip);
        // addLogEntry(camera, "Blur", event, "", "");
        recentBlurLogKeys.insert(key);
    }
    else if (type == "anomaly_status") {
        QString status = data["status"].toString();
        QString timestamp = data["timestamp"].toString();

        qDebug() << "[이상소음 상태]" << status << "at" << timestamp;

        if (status == "detected" && lastAnomalyStatus[camera.name] != "detected") {
            addLogEntry(camera.name, "Sound", "⚠️ 이상소음 감지됨", "", "이상소음 발생", camera.ip);
        }
        else if (status == "cleared" && lastAnomalyStatus[camera.name] == "detected") {
            addLogEntry(camera.name, "Sound", "✅ 이상소음 해제됨", "", "이상소음 정상 상태", camera.ip);
        }

        lastAnomalyStatus[camera.name] = status;
    }
    else if (type == "new_fall") {
        QString ts = data["timestamp"].toString();
        int count = data["count"].toInt();

        if (count > 0) {
            QString event = "🚨 낙상 감지";
            QString details = QString("낙상 감지 시각: %1").arg(ts);

            addLogEntry(camera.name, "Fall", event, "", details, camera.ip);

            /*
            // (선택) 팝업 알림도 추가 가능
            QMessageBox *popup = new QMessageBox(this);
            popup->setIcon(QMessageBox::Warning);
            popup->setWindowTitle("낙상 감지");
            popup->setText(QString("%1 카메라에서 낙상이 감지되었습니다!\n시각: %2").arg(camera.name, ts));
            popup->setStandardButtons(QMessageBox::Ok);
            popup->setModal(false);
            popup->show();
            */
        }
    }
    else if (type == "stm_status_update") {
        qDebug() << "[STM 상태 응답 수신]" << data;
        double temp = data["temperature"].toDouble();
        int light = data["light"].toInt();
        bool buzzer = data["buzzer_on"].toBool();
        bool led = data["led_on"].toBool();

        QString details = QString("🌡️ 온도: %1°C | 💡 밝기: %2 | 🔔 버저: %3 | 💡 LED: %4")
                              .arg(temp, 0, 'f', 2)
                              .arg(light)
                              .arg(buzzer ? "ON" : "OFF")
                              .arg(led ? "ON" : "OFF");

        healthCheckResponded.insert(camera.ip);  // ✅ 응답 확인 기록
        addLogEntry(camera.name, "Health", "✅ 상태 수신", "", details, camera.ip);
    }
    else if (type == "mode_change_ack") {
        QString status = obj["status"].toString();
        QString mode = obj["mode"].toString();
        QString message = obj["message"].toString();

        if (status == "error") {
            qWarning() << "[모드 변경 실패]" << message;
            QMessageBox::warning(this, "모드 변경 실패", message);
        } else {
            qDebug() << "[모드 변경 성공 응답]" << mode;
        }
    }
    else {
        qWarning() << "[WebSocket] 알 수 없는 타입 수신:" << type;
    }
}


void MainWindow::onSocketConnected() {
    qDebug() << "[웹소켓] 연결됨";
}
void MainWindow::onSocketDisconnected() {
    qDebug() << "[웹소켓] 해제됨";
}
void MainWindow::onSocketErrorOccurred(QAbstractSocket::SocketError error) {
    qDebug() << "[웹소켓 오류]" << error;
}

void MainWindow::loadInitialLogs()
{
    fullLogEntries.clear();  // 기존 로그 초기화

    for (const CameraInfo &camera : cameraList) {
        QString urlPPE = QString("https://%1:8443/api/detections").arg(camera.ip);  // HTTPS 수정도 반영

        QNetworkRequest reqPPE{QUrl(urlPPE)};
        QNetworkReply *replyPPE = networkManager->get(reqPPE);
        replyPPE->ignoreSslErrors();  // 자가서명 무시

        connect(replyPPE, &QNetworkReply::finished, this, [=]() {
            replyPPE->deleteLater();

            if (replyPPE->error() != QNetworkReply::NoError) {
                qWarning() << "[로그 요청 실패]" << camera.ip << ":" << replyPPE->errorString();
                return;
            }

            QByteArray raw = replyPPE->readAll();

            QJsonDocument doc = QJsonDocument::fromJson(raw);
            if (!doc.isObject()) {
                qWarning() << "[JSON 파싱 실패]" << camera.ip;
                return;
            }

            QJsonArray arr = doc["detections"].toArray();

            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                int person = obj["person_count"].toInt();
                int helmet = obj["helmet_count"].toInt();
                int vest = obj["safety_vest_count"].toInt();
                double conf = obj["avg_confidence"].toDouble();
                QString ts = obj["timestamp"].toString();
                QString imgPath = obj["image_path"].toString();

                QString event;
                if (helmet < person && vest >= person)
                    event = "⛑️ 헬멧 미착용 감지";
                else if (vest < person && helmet >= person)
                    event = "🦺 조끼 미착용 감지";
                else
                    event = "⛑️ 🦺 PPE 미착용 감지";

                QString detail = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                                     .arg(person).arg(helmet).arg(vest).arg(conf, 0, 'f', 2);

                fullLogEntries.append({
                    camera.name, "PPE", event, imgPath, detail,
                    ts.left(10), ts.mid(11, 8),
                    static_cast<int>(cameraList.indexOf(camera)) + 1, camera.ip
                });
            }

        });
    }
}

void MainWindow::performHealthCheck()
{
    healthCheckResponded.clear();
    for (const CameraInfo &camera : cameraList) {
        if (socketMap.contains(camera.ip)) {
            QWebSocket *socket = socketMap[camera.ip];
            QJsonObject req;
            req["type"] = "request_stm_status";
            QJsonDocument doc(req);
            socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));

            healthCheckRequestTime[camera.ip] = QDateTime::currentDateTime();
            qDebug() << "[헬시체크 요청 전송]" << camera.ip;

            // ✅ 5초 후 응답 없으면 경고 로그 추가
            QTimer::singleShot(5000, this, [=]() {
                if (!healthCheckResponded.contains(camera.ip)) {
                    addLogEntry(camera.name, "Health", "⚠️ 헬시체크 응답 없음", "", "STM 상태 응답이 5초 내 도착하지 않았습니다", camera.ip);
                }
            });
        } else {
            addLogEntry(camera.name, "Health", "❌ 웹소켓 없음", "", "웹소켓 연결이 없어 상태 요청 불가", camera.ip);
        }
    }
}


