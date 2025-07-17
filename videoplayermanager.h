#ifndef VIDEOPLAYERMANAGER_H
#define VIDEOPLAYERMANAGER_H

#include "camerainfo.h"

#include <QObject>
#include <QVector>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QGridLayout>

class VideoPlayerManager : public QObject
{
    Q_OBJECT

public:
    explicit VideoPlayerManager(QObject *parent = nullptr);
    ~VideoPlayerManager();

    void clearPlayers();
    void setupVideoGrid(QGridLayout *layout, const QVector<CameraInfo> &cameraList, const QString &streamSuffix);
    void switchStreamForAllPlayers(const QVector<CameraInfo> &cameraList, const QString &suffix);

private:
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;
};

#endif // VIDEOPLAYERMANAGER_H
