#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QWidget>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTableWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QNetworkAccessManager>

class CameraListDialog;

struct CameraInfo {
    QString name;
    QString ip;
    QString port;

    QString rtspUrl() const {
        return QString("rtsps://%1:%2/raw").arg(ip, port);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refreshVideoGrid();

private slots:
    void onCameraListClicked();
    void onLogHistoryClicked();
    void sendModeChangeRequest(const QString &mode, const CameraInfo &camera);
    void pollLogsFromServer();

private:
    void setupUI();
    void addLogEntry(const QString &camera, const QString &alert);

    QVector<CameraInfo> cameraList;
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;
    QVector<QPair<QString, QString>> fullLogEntries; // (Camera, Alert)

    QWidget *centralWidget;
    QWidget *videoArea;
    QGridLayout *videoGridLayout;
    QScrollArea *scrollArea;
    QTableWidget *logTable;

    QPushButton *cameraListButton;

    QCheckBox *rawCheckBox;
    QCheckBox *blurCheckBox;
    QCheckBox *ppeDetectorCheckBox;

    QString lastPpeTimestamp;  // 최신 PPE 로그 시간
    QString lastBlurTimestamp; // 최신 Blur 로그 시간 (추가해도 좋음)

    void switchStreamForAllPlayers(const QString &suffix);

    CameraListDialog *cameraListDialog = nullptr;
    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H

