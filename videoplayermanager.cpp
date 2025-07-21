#include "videoplayermanager.h"
#include <QLabel>

VideoPlayerManager::VideoPlayerManager(QObject *parent)
    : QObject(parent)
{
}

VideoPlayerManager::~VideoPlayerManager()
{
    clearPlayers();
}

void VideoPlayerManager::clearPlayers()
{
    for (QMediaPlayer* player : players) {
        player->stop();
        delete player;
    }
    players.clear();

    for (QVideoWidget* widget : videoWidgets) {
        delete widget;
    }
    videoWidgets.clear();
}

void VideoPlayerManager::setupVideoGrid(QGridLayout *layout, const QVector<CameraInfo> &cameraList, const QString &streamSuffix)
{
    // 기존 레이아웃 초기화
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    clearPlayers();

    int total = std::max(4, static_cast<int>(cameraList.size()));
    int columns = 2;
    int rows = (total + 1) / 2;

    for (int i = 0; i < total; ++i) {
        QWidget *videoFrame = new QWidget();
        videoFrame->setFixedSize(320, 240);
        videoFrame->setStyleSheet("background-color: black;");

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
            QVBoxLayout *noCamLayout = new QVBoxLayout(videoFrame);
            QLabel *noCam = new QLabel("No Camera");
            noCam->setAlignment(Qt::AlignCenter);
            noCam->setStyleSheet("color: white;");
            noCamLayout->addWidget(noCam);
            videoFrame->setLayout(noCamLayout);
        }

        layout->addWidget(videoFrame, i / columns, i % columns);
    }
}

void VideoPlayerManager::switchStreamForAllPlayers(const QVector<CameraInfo> &cameraList, const QString &suffix)
{
    for (int i = 0; i < cameraList.size() && i < players.size(); ++i) {
        QString streamUrl = QString("rtsps://%1:%2/%3")
        .arg(cameraList[i].ip)
            .arg(cameraList[i].port)
            .arg(suffix);

        players[i]->stop();
        players[i]->setSource(QUrl(streamUrl));
        players[i]->play();
    }
}
