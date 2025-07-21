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
    setMinimumSize(1500, 800);

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
    setCentralWidget(centralWidget);

    setupVideoSection();
    setupTopBar();
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

void MainWindow::setupVideoSection() {
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
    scrollArea->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addLayout(streamingHeaderLayout);
    videoLayout->addWidget(scrollArea);

    videoSection = new QWidget();
    videoSection->setLayout(videoLayout);
    videoSection->setFixedWidth(640 + 20);
}

void MainWindow::setupLogSection() {
    QLabel *alertLabel = new QLabel("Alert");
    alertLabel->setStyleSheet("font-weight: bold; color: orange;");

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

    // Raw 체크박스
    connect(rawCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        // Raw는 해제되지 않도록 강제 복원
        if (!checked) {
            rawCheckBox->blockSignals(true);
            rawCheckBox->setChecked(true);
            rawCheckBox->blockSignals(false);
            return;
        }

        // 나머지 모드는 해제하고 Raw 적용
        blurCheckBox->blockSignals(true);
        ppeDetectorCheckBox->blockSignals(true);
        blurCheckBox->setChecked(false);
        ppeDetectorCheckBox->setChecked(false);
        blurCheckBox->blockSignals(false);
        ppeDetectorCheckBox->blockSignals(false);

        for (const CameraInfo &camera : cameraList)
            sendModeChangeRequest("raw", camera);

        // switchStreamForAllPlayers("raw");
        switchStreamForAllPlayers("processed");
        addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
    });

    // Blur 체크박스
    connect(blurCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true);
            ppeDetectorCheckBox->blockSignals(true);
            rawCheckBox->setChecked(false);
            ppeDetectorCheckBox->setChecked(false);
            rawCheckBox->blockSignals(false);
            ppeDetectorCheckBox->blockSignals(false);

            // 모든 카메라에 blur 모드 전송
            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("blur", camera);

            switchStreamForAllPlayers("processed");
            addLogEntry("System", "Blur", "Blur mode enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()) {
                // 이미 Raw가 체크된 상태면 생략
                if (!rawCheckBox->isChecked()) {
                    rawCheckBox->blockSignals(true);
                    rawCheckBox->setChecked(true);
                    rawCheckBox->blockSignals(false);

                    for (const CameraInfo &camera : cameraList)
                        sendModeChangeRequest("raw", camera);

                    switchStreamForAllPlayers("raw");
                    addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
                }
            }
        }
    });

    // PPE Detector 체크박스
    connect(ppeDetectorCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            rawCheckBox->blockSignals(true);
            blurCheckBox->blockSignals(true);
            rawCheckBox->setChecked(false);
            blurCheckBox->setChecked(false);
            rawCheckBox->blockSignals(false);
            blurCheckBox->blockSignals(false);

            // 모든 카메라에 detect 모드 전송
            for (const CameraInfo &camera : cameraList)
                sendModeChangeRequest("detect", camera);

            switchStreamForAllPlayers("processed");
            addLogEntry("System", "PPE", "PPE Detector enabled", "", "", "");
        } else {
            if (!rawCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()) {
                // 이미 Raw가 체크된 상태면 생략
                if (!rawCheckBox->isChecked()) {
                    rawCheckBox->blockSignals(true);
                    rawCheckBox->setChecked(true);
                    rawCheckBox->blockSignals(false);

                    for (const CameraInfo &camera : cameraList)
                        sendModeChangeRequest("raw", camera);

                    switchStreamForAllPlayers("raw");
                    addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
                }
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
    functionLayout->addStretch();

    functionLayout->addWidget(healthCheckButton);

    functionSection = new QWidget();
    functionSection->setLayout(functionLayout);
    functionSection->setFixedWidth(200);
}

void MainWindow::setupMainLayout() {
    QHBoxLayout *mainBodyLayout = new QHBoxLayout();
    mainBodyLayout->addWidget(videoSection);
    mainBodyLayout->addWidget(logSection);
    mainBodyLayout->addWidget(functionSection);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(mainBodyLayout);
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

    // 현재 체크박스 상태 기준으로 스트림 suffix 결정
    // QString streamSuffix = "raw";
    QString streamSuffix = "processed";
    if (blurCheckBox->isChecked() || ppeDetectorCheckBox->isChecked()) {
        streamSuffix = "processed";
    }

    // 스트리밍 구성은 VideoPlayerManager에게 위임
    videoPlayerManager->setupVideoGrid(videoGridLayout, cameraList, streamSuffix);

    // 모든 카메라가 삭제된 경우: 체크박스 초기화
    if (cameraList.isEmpty()) {
        rawCheckBox->blockSignals(true);
        blurCheckBox->blockSignals(true);
        ppeDetectorCheckBox->blockSignals(true);

        rawCheckBox->setChecked(false);
        blurCheckBox->setChecked(false);
        ppeDetectorCheckBox->setChecked(false);

        rawCheckBox->blockSignals(false);
        blurCheckBox->blockSignals(false);
        ppeDetectorCheckBox->blockSignals(false);
    }

    // 카메라가 있고 아무 모드도 선택 안되어 있을 경우 → Raw 적용
    if (!cameraList.isEmpty() && !blurCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()) {
        rawCheckBox->blockSignals(true);
        rawCheckBox->setChecked(true);
        rawCheckBox->blockSignals(false);

        for (const CameraInfo &camera : cameraList)
            sendModeChangeRequest("raw", camera);

        // switchStreamForAllPlayers("raw");
        switchStreamForAllPlayers("processed");

        addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
    }

    setupWebSocketConnections();
    loadInitialLogs();  // 카메라 재정렬 이후 초기 로그 불러오기
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
    if (camera.ip.isEmpty() || camera.port.isEmpty()) {
        qWarning() << "[모드 변경] 카메라 IP 또는 포트 정보 없음 →" << camera.name;
        return;
    }

    QString apiUrl = QString("https://%1:8443/api/mode").arg(camera.ip);
    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["mode"] = mode;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, data);
    reply->ignoreSslErrors();

    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
            if (!responseDoc.isObject()) {
                qWarning() << "[모드 변경] 응답 JSON 파싱 실패 →" << camera.name;
                return;
            }

            QJsonObject obj = responseDoc.object();
            QString status = obj["status"].toString();
            QString message = obj["message"].toString();

            if (status != "success") {
                qWarning() << "[모드 변경 실패]" << camera.name << ":" << message;
            } else {
                qDebug() << "[모드 변경 성공]" << camera.name << "→" << mode;
            }
        } else {
            qWarning() << "[모드 변경 네트워크 오류]" << camera.name << ":" << reply->errorString();
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
        if (helmet < person && vest >= person)
            event = "⛑️ 헬멧 미착용 감지";
        else if (vest < person && helmet >= person)
            event = "🦺 조끼 미착용 감지";
        else
            event = "⛑️ 🦺 PPE 미착용 감지";

        QString details = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                              .arg(person).arg(helmet).arg(vest).arg(conf, 0, 'f', 2);

        qDebug() << "[PPE 이벤트]" << event << "IP:" << camera.ip;

        addLogEntry(camera.name, "PPE", event, imagePath, details, camera.ip);
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

        addLogEntry(camera.name, "Health", "✅ 상태 수신", "", details, camera.ip);
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
    for (const CameraInfo &camera : cameraList) {
        if (socketMap.contains(camera.ip)) {
            QWebSocket *socket = socketMap[camera.ip];
            QJsonObject req;
            req["type"] = "request_stm_status";
            QJsonDocument doc(req);
            socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
            qDebug() << "[헬시체크 요청 전송]" << camera.ip;
        } else {
            qWarning() << "[헬시체크 실패] 웹소켓 없음:" << camera.ip;
        }
    }
}
