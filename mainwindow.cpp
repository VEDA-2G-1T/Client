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
    setupUI();
    setWindowTitle("Smart SafetyNet");
    setMinimumSize(1500, 800);

    // REST API 통신용
    networkManager = new QNetworkAccessManager(this);

    /*
    QTimer *logTimer = new QTimer(this);
    connect(logTimer, &QTimer::timeout, this, &MainWindow::pollLogsFromServer);
    logTimer->start(2000);
    */

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

    QVBoxLayout *videoLayout = new QVBoxLayout();
    videoLayout->addLayout(streamingHeaderLayout);
    videoLayout->addWidget(scrollArea);

    videoSection = new QWidget();
    videoSection->setLayout(videoLayout);
    videoSection->setFixedWidth(640);
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

    // ✅ Raw 체크박스
    connect(rawCheckBox, &QCheckBox::toggled, this, [=](bool checked) {
        // ✅ Raw는 해제되지 않도록 강제 복원
        if (!checked) {
            rawCheckBox->blockSignals(true);
            rawCheckBox->setChecked(true);
            rawCheckBox->blockSignals(false);
            return;
        }

        // ✅ 나머지 모드는 해제하고 Raw 적용
        blurCheckBox->blockSignals(true);
        ppeDetectorCheckBox->blockSignals(true);
        blurCheckBox->setChecked(false);
        ppeDetectorCheckBox->setChecked(false);
        blurCheckBox->blockSignals(false);
        ppeDetectorCheckBox->blockSignals(false);

        for (const CameraInfo &camera : cameraList)
            sendModeChangeRequest("raw", camera);

        switchStreamForAllPlayers("raw");
        addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
    });

    // ✅ Blur 체크박스
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
                // ✅ 이미 Raw가 체크된 상태면 생략
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

    // ✅ PPE Detector 체크박스
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
                // ✅ 이미 Raw가 체크된 상태면 생략
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

    QVBoxLayout *functionLayout = new QVBoxLayout();
    functionLayout->addWidget(functionLabelButton);
    functionLayout->addWidget(rawCheckBox);
    functionLayout->addWidget(blurCheckBox);
    functionLayout->addWidget(ppeDetectorCheckBox);
    functionLayout->addStretch();

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

// refreshVideoGrid()
void MainWindow::refreshVideoGrid()
{
    // 레이아웃 초기화
    QLayoutItem *child;
    while ((child = videoGridLayout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    // 기존 플레이어 제거
    for (QMediaPlayer *player : players) {
        player->stop();
        delete player;
    }
    players.clear();
    videoWidgets.clear();

    // 화면 크기 조정
    int total = std::max(4, static_cast<int>(cameraList.size()));
    int columns = 2;
    int rows = (total + 1) / 2;
    videoArea->setMinimumSize(columns * 320, rows * 240);

    // 현재 체크박스 상태 기준으로 스트림 suffix 결정
    QString streamSuffix = "raw";
    if (blurCheckBox->isChecked() || ppeDetectorCheckBox->isChecked()) {
        streamSuffix = "processed";
    }

    // 카메라 별 영상 위젯 배치
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

            QString url = QString("rtsps://%1:%2/%3")
                              .arg(cameraList[i].ip)
                              .arg(cameraList[i].port)
                              .arg(streamSuffix);
            player->setSource(QUrl(url));
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

    // ✅ 모든 카메라가 삭제된 경우: 체크박스 초기화
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

    // ✅ 카메라가 있고 아무 모드도 선택 안되어 있을 경우 → Raw 적용
    if (!cameraList.isEmpty() && !blurCheckBox->isChecked() && !ppeDetectorCheckBox->isChecked()) {
        rawCheckBox->blockSignals(true);
        rawCheckBox->setChecked(true);
        rawCheckBox->blockSignals(false);

        for (const CameraInfo &camera : cameraList)
            sendModeChangeRequest("raw", camera);

        switchStreamForAllPlayers("raw");

        addLogEntry("System", "Raw", "Raw mode enabled", "", "", "");
    }
    setupWebSocketConnections();
    loadInitialLogs();  // ✅ 카메라 재정렬 이후 초기 로그 불러오기
}



void MainWindow::addLogEntry(const QString &cameraName, const QString &event,
                             const QString &imagePath, const QString &details, const QString &ip)
{
    QString function = event.contains("Blur") ? "Blur" : "PPE";
    int zone = -1;

    for (int i = 0; i < cameraList.size(); ++i) {
        if (cameraList[i].name == cameraName) {
            zone = i + 1;
            break;
        }
    }

    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QString time = QTime::currentTime().toString("HH:mm:ss");

    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(cameraName));
    logTable->setItem(0, 1, new QTableWidgetItem(date));  // ✅ Date
    logTable->setItem(0, 2, new QTableWidgetItem(time));  // ✅ Time
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

void MainWindow::addLogEntry(const CameraInfo &camera, const QString &event,
                             const QString &imagePath, const QString &details)
{
    addLogEntry(camera.name, event, imagePath, details, camera.ip);
}

void MainWindow::addLogEntry(const CameraInfo &camera, const QString &function,
                             const QString &event, const QString &imagePath, const QString &details)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QString time = QTime::currentTime().toString("HH:mm:ss");

    int zone = -1;
    for (int i = 0; i < cameraList.size(); ++i) {
        if (cameraList[i].name == camera.name) {
            zone = i + 1;
            break;
        }
    }

    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(camera.name));
    logTable->setItem(0, 1, new QTableWidgetItem(date));  // ✅ Date
    logTable->setItem(0, 2, new QTableWidgetItem(time));  // ✅ Time
    logTable->setItem(0, 3, new QTableWidgetItem(function));
    logTable->setItem(0, 4, new QTableWidgetItem(event));

    fullLogEntries.prepend({
        camera.name,
        function,
        event,
        imagePath,
        details,
        date,
        time,
        zone,
        camera.ip
    });

    if (logTable->rowCount() > 20)
        logTable->removeRow(logTable->rowCount() - 1);
}

// (4) 카메라 이름 기반 – function 수동 지정 ← ✅ 새로 추가한 함수
void MainWindow::addLogEntry(const QString &cameraName,
                             const QString &function,
                             const QString &event,
                             const QString &imagePath,
                             const QString &details,
                             const QString &ip)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QString time = QTime::currentTime().toString("HH:mm:ss");

    logTable->insertRow(0);
    logTable->setItem(0, 0, new QTableWidgetItem(cameraName));
    logTable->setItem(0, 1, new QTableWidgetItem(date));  // ✅ Date
    logTable->setItem(0, 2, new QTableWidgetItem(time));  // ✅ Time
    logTable->setItem(0, 3, new QTableWidgetItem(function));
    logTable->setItem(0, 4, new QTableWidgetItem(event));

    fullLogEntries.prepend({cameraName, function, event, imagePath, details, date, time, -1, ip});

    if (logTable->rowCount() > 20)
        logTable->removeRow(logTable->rowCount() - 1);
}


void MainWindow::onLogHistoryClicked()
{
    LogHistoryDialog dialog(this, &fullLogEntries);  // ✅ 로그 목록 전달
    dialog.exec();
}

void MainWindow::sendModeChangeRequest(const QString &mode, const CameraInfo &camera)
{
    if (camera.ip.isEmpty() || camera.port.isEmpty()) {
        qWarning() << "[모드 변경] 카메라 IP 또는 포트 정보 없음 →" << camera.name;
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

void MainWindow::pollLogsFromServer()
{
    if (cameraList.isEmpty()) return;

    for (const CameraInfo &camera : cameraList) {
        //
        // ✅ [1] 이상소음 감지 요청 (모드와 관계없이 항상 수행)
        //
        QString anomalyUrl = QString("http://%1/api/anomaly/status").arg(camera.ip);
        QNetworkRequest anomalyRequest{QUrl(anomalyUrl)};
        QNetworkReply *anomalyReply = networkManager->get(anomalyRequest);

        connect(anomalyReply, &QNetworkReply::finished, this, [=]() {
            anomalyReply->deleteLater();

            if (anomalyReply->error() != QNetworkReply::NoError)
                return;

            QJsonDocument doc = QJsonDocument::fromJson(anomalyReply->readAll());
            if (doc.isNull() || !doc.isObject())
                return;

            QString status = doc["status"].toString();
            if (status == "detected" && lastAnomalyStatus[camera.name] != "detected") {
                QString event = "⚠️ 이상소음 감지됨";
                QString details = "이상소음이 감지되어 경고를 발생시킴";
                addLogEntry(camera, "Sound", event, "", details);
            }

            lastAnomalyStatus[camera.name] = status;
        });

        //
        // ✅ [2] PPE / Blur 감지 요청 (체크박스에 따라 조건적 수행)
        //
        QString baseUrl = QString("http://%1").arg(camera.ip);
        QString endpoint;

        if (ppeDetectorCheckBox->isChecked()) {
            endpoint = "/api/detections";
        } else if (blurCheckBox->isChecked()) {
            endpoint = "/api/blur";
        } else {
            continue;  // PPE/Blur 요청 생략, 이상소음 요청은 이미 위에서 수행됨
        }

        QUrl url(baseUrl + endpoint);
        QNetworkRequest request(url);
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply, camera]() {
            QByteArray rawData = reply->readAll();
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError)
                return;

            QJsonDocument doc = QJsonDocument::fromJson(rawData);
            if (doc.isNull() || !doc.isObject()) return;

            QJsonObject root = doc.object();
            if (root["status"].toString() != "success") return;

            // ✅ PPE 로그 처리
            if (root.contains("detections")) {
                QJsonArray arr = root["detections"].toArray();
                for (const QJsonValue &val : arr) {
                    QJsonObject obj = val.toObject();
                    QString ts = obj["timestamp"].toString();

                    if (!lastPpeTimestamps[camera.name].isEmpty() &&
                        ts <= lastPpeTimestamps[camera.name])
                        continue;

                    int personCount = obj["person_count"].toInt();
                    int helmetCount = obj["helmet_count"].toInt();
                    int vestCount = obj["safety_vest_count"].toInt();
                    double confidence = obj["avg_confidence"].toDouble();

                    QString event;
                    if (helmetCount == vestCount && personCount <= helmetCount)
                        return;
                    else if (helmetCount < vestCount)
                        event = "⛑️ 헬멧 미착용 감지";
                    else if (helmetCount > vestCount)
                        event = "🦺 조끼 미착용 감지";
                    else
                        event = "⛑️ 🦺 PPE 미착용 감지";

                    QString detail = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                                         .arg(personCount)
                                         .arg(helmetCount)
                                         .arg(vestCount)
                                         .arg(confidence, 0, 'f', 2);

                    QString imgPath = obj["image_path"].toString();
                    addLogEntry(camera.name, event, imgPath, detail, camera.ip);

                    lastPpeTimestamps[camera.name] = ts;
                }
            }

            // ✅ Blur 로그 처리
            if (root.contains("person_counts")) {
                QJsonArray arr = root["person_counts"].toArray();

                for (const QJsonValue &val : arr) {
                    QJsonObject obj = val.toObject();
                    QString ts = obj["timestamp"].toString();
                    QString logKey = camera.name + "_" + ts;

                    if (recentBlurLogKeys.contains(logKey))
                        continue;

                    int personCount = 0;
                    if (obj["count"].isDouble()) {
                        personCount = obj["count"].toInt();
                    } else if (obj["count"].isString()) {
                        personCount = obj["count"].toString().toInt();
                    } else {
                        qWarning() << "[Blur 로그] count 타입 이상 →" << obj["count"];
                    }

                    if (personCount > 0) {
                        recentBlurLogKeys.insert(logKey);
                        if (recentBlurLogKeys.size() > 1000) {
                            auto it = recentBlurLogKeys.begin();
                            for (int i = 0; i < 200 && it != recentBlurLogKeys.end(); ++i)
                                it = recentBlurLogKeys.erase(it);
                        }

                        QString event = QString("🔍 %1명 감지").arg(personCount);
                        addLogEntry(camera, "Blur", event, "", "");
                        lastBlurTimestamps[camera.name] = ts;
                        break;  // 👉 유효한 로그 1개만 등록
                    }
                }
            }
        });
    }
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

    // ✅ 상대 경로일 경우 "./" 제거
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
        connect(socket, &QWebSocket::connected, this, &MainWindow::onSocketConnected);
        connect(socket, &QWebSocket::disconnected, this, &MainWindow::onSocketDisconnected);
        /*connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                this, &MainWindow::onSocketErrorOccurred);*/
        connect(socket, &QWebSocket::errorOccurred,
                this, &MainWindow::onSocketErrorOccurred);
        connect(socket, &QWebSocket::textMessageReceived,
                this, &MainWindow::onSocketMessageReceived);

        QString wsUrl = QString("ws://%1/ws").arg(camera.ip);
        socket->open(QUrl(wsUrl));
        socketMap[camera.ip] = socket;
    }
}


void MainWindow::onSocketMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QJsonObject data = obj["data"].toObject();

    QString ipSender;
    QWebSocket *senderSocket = qobject_cast<QWebSocket*>(sender());
    for (auto it = socketMap.begin(); it != socketMap.end(); ++it) {
        if (it.value() == senderSocket) {
            ipSender = it.key();
            break;
        }
    }
    if (ipSender.isEmpty()) return;

    const CameraInfo *cameraPtr = nullptr;
    for (const CameraInfo &cam : cameraList) {
        if (cam.ip == ipSender) {
            cameraPtr = &cam;
            break;
        }
    }
    if (!cameraPtr) return;
    const CameraInfo &camera = *cameraPtr;

    if (type == "new_detection") {
        int person = data["person_count"].toInt();
        int helmet = data["helmet_count"].toInt();
        int vest = data["safety_vest_count"].toInt();
        double conf = data["avg_confidence"].toDouble();
        QString imagePath = data["image_path"].toString();
        QString ts = data["timestamp"].toString();

        QString event;
        if (helmet < person && vest >= helmet)
            event = "⛑️ 헬멧 미착용 감지";
        else if (vest < person && helmet >= vest)
            event = "🦺 조끼 미착용 감지";
        else
            event = "⛑️ 🦺 PPE 미착용 감지";

        QString details = QString("👷 %1명 | ⛑️ %2명 | 🦺 %3명 | 신뢰도: %4")
                              .arg(person).arg(helmet).arg(vest).arg(conf, 0, 'f', 2);
        addLogEntry(camera.name, "PPE", event, imagePath, details, camera.ip);
    }
    else if (type == "new_blur") {
        QString ts = data["timestamp"].toString();
        QString key = camera.name + "_" + ts;
        if (recentBlurLogKeys.contains(key)) return;

        int count = data["count"].toInt();
        QString event = QString("🔍 %1명 감지").arg(count);
        addLogEntry(camera, "Blur", event, "", "");
        recentBlurLogKeys.insert(key);
    }
    else if (type == "anomaly_status") {
        QString status = data["status"].toString();
        QString timestamp = data["timestamp"].toString();  // 예: "2025-07-15 10:01:10"

        if (status == "detected" && lastAnomalyStatus[camera.name] != "detected") {
            addLogEntry(camera.name, "Sound", "⚠️ 이상소음 감지됨", "", "이상소음 발생", camera.ip);
        }
        else if (status == "cleared" && lastAnomalyStatus[camera.name] == "detected") {
            addLogEntry(camera.name, "Sound", "✅ 이상소음 해제됨", "", "이상소음 정상 상태", camera.ip);
        }

        lastAnomalyStatus[camera.name] = status;
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
/*
void MainWindow::loadInitialLogs()
{
    fullLogEntries.clear();  // 기존 로그 초기화

    for (const CameraInfo &camera : cameraList) {
        QString urlStr = QString("http://%1/api/logs").arg(camera.ip);
        QNetworkRequest request{QUrl(urlStr)};
        QNetworkReply *reply = networkManager->get(request);

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "[초기 로그 로딩 실패]" << camera.name << reply->errorString();
                return;
            }

            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

            qDebug() << "[요청 URL]" << urlStr;
            qDebug() << "[응답 JSON]" << doc;

            if (!doc.isArray()) {
                qWarning() << "[초기 로그 응답 형식 이상]" << camera.name;
                return;
            }

            QJsonArray arr = doc.array();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();

                LogEntry entry;
                entry.camera = camera.name;
                entry.function = obj["function"].toString();
                entry.alert = obj["event"].toString();
                entry.imagePath = obj["image_path"].toString();
                entry.details = obj["details"].toString();
                entry.date = obj["date"].toString();
                entry.time = obj["time"].toString();
                entry.zone = cameraList.indexOf(camera) + 1;
                entry.ip = camera.ip;

                fullLogEntries.append(entry);
            }

            qDebug() << "[초기 로그 로딩 완료]:" << arr.size() << "개 from" << camera.name;
        });
    }
}
*/

void MainWindow::loadInitialLogs()
{
    fullLogEntries.clear();  // 기존 로그 초기화

    for (const CameraInfo &camera : cameraList) {
        QString urlPPE = QString("http://%1/api/detections").arg(camera.ip);
        QNetworkRequest reqPPE{QUrl(urlPPE)};
        QNetworkReply *replyPPE = networkManager->get(reqPPE);
        connect(replyPPE, &QNetworkReply::finished, this, [=]() {
            replyPPE->deleteLater();
            if (replyPPE->error() != QNetworkReply::NoError) return;

            QJsonDocument doc = QJsonDocument::fromJson(replyPPE->readAll());
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
                if (helmet < person && vest >= helmet)
                    event = "⛑️ 헬멧 미착용 감지";
                else if (vest < person && helmet >= vest)
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
        /*
        // (2) Blur 로그 요청
        QString urlBlur = QString("http://%1/api/blur").arg(camera.ip);
        QNetworkRequest reqBlur{QUrl(urlBlur)};
        QNetworkReply *replyBlur = networkManager->get(reqBlur);
        connect(replyBlur, &QNetworkReply::finished, this, [=]() {
            replyBlur->deleteLater();
            if (replyBlur->error() != QNetworkReply::NoError) return;

            QJsonDocument doc = QJsonDocument::fromJson(replyBlur->readAll());
            QJsonArray arr = doc["person_counts"].toArray();
            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                int count = obj["count"].toInt();
                QString ts = obj["timestamp"].toString();

                QString event = QString("🔍 %1명 감지").arg(count);
                fullLogEntries.append({
                    camera.name, "Blur", event, "", "",
                    ts.left(10), ts.mid(11, 8),
                    cameraList.indexOf(camera) + 1, camera.ip
                });
            }
        });

        // (3) 이상소음 상태 요청
        QString urlAnomaly = QString("http://%1/api/anomaly/status").arg(camera.ip);
        QNetworkRequest reqAnomaly{QUrl(urlAnomaly)};
        QNetworkReply *replyAnomaly = networkManager->get(reqAnomaly);
        connect(replyAnomaly, &QNetworkReply::finished, this, [=]() {
            replyAnomaly->deleteLater();
            if (replyAnomaly->error() != QNetworkReply::NoError) return;

            QJsonDocument doc = QJsonDocument::fromJson(replyAnomaly->readAll());
            QString status = doc["status"].toString();
            QString ts = doc["timestamp"].toString();

            if (status == "detected") {
                fullLogEntries.append({
                    camera.name, "Sound", "⚠️ 이상소음 감지됨", "", "이상소음이 감지되었습니다.",
                    ts.left(10), ts.mid(11, 8),
                    cameraList.indexOf(camera) + 1, camera.ip
                });
            }
        });
    */
    }
}
