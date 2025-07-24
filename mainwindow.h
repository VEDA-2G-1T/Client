#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videoplayermanager.h"
#include "camerainfo.h"

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
#include <QSet>  // 이 줄 추가!
#include <QWebSocket>
#include <QMap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>

class CameraListDialog;

// logentry.h 또는 mainwindow.h 내부 등 구조체 선언부에 아래처럼 추가
struct LogEntry {
    QString camera;
    QString function;   // 명시적 필드 추가
    QString alert;
    QString imagePath;
    QString details;
    QString date;
    QString time;
    int zone;  // 실제 스트리밍 영역 번호
    QString ip; // IP 필드 추가

};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setupOnvifSection();
    void refreshVideoGrid();
    QSet<QString> recentBlurLogKeys;  // 중복 Blur 로그 방지용 키



private slots:
    void onCameraListClicked();
    void onLogHistoryClicked();
    void sendModeChangeRequest(const QString &mode, const CameraInfo &camera);
    void onAlertItemClicked(int row, int column);
    void performHealthCheck();

private:
    void setupUI();
    void setupTopBar();
    void setupVideoSection();
    void setupLogSection();
    void setupFunctionPanel();
    void setupMainLayout();
    void addLogEntry(const QString &cameraName,
                     const QString &function,
                     const QString &event,
                     const QString &imagePath,
                     const QString &details,
                     const QString &ip);
    void loadInitialLogs();

    QHBoxLayout *topLayout;
    QWidget *onvifSection;  // onvifSection 위젯
    QWidget *videoSection;
    QWidget *logSection;
    QWidget *functionSection;

    void setupWebSocketConnections();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketMessageReceived(const QString &message);
    void onSocketErrorOccurred(QAbstractSocket::SocketError error);

    QMap<QString, int> ppeViolationStreakMap;

    QMap<QString, QDateTime> healthCheckRequestTime;  // IP → 요청 보낸 시각
    QSet<QString> healthCheckResponded;

    VideoPlayerManager *videoPlayerManager = nullptr;

    QVector<CameraInfo> cameraList;
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;
    QVector<LogEntry> fullLogEntries;
    QMap<QString, QString> lastAnomalyStatus;

    QMediaPlayer* onvifPlayer = nullptr;
    QVideoWidget* onvifVideo = nullptr;
    QWidget* onvifFrame = nullptr;

    QWidget *centralWidget;
    QWidget *videoArea;
    QGridLayout *videoGridLayout;
    QScrollArea *scrollArea;
    QTableWidget *logTable;

    QPushButton *cameraListButton;

    QCheckBox *rawCheckBox;
    QCheckBox *blurCheckBox;
    QCheckBox *ppeDetectorCheckBox;
    QCheckBox *nightIntrusionCheckBox;
    QCheckBox *fallDetectionCheckBox;  // 🔍 낙상 감지 모드

    QMap<QString, QString> lastPpeTimestamps;
    QMap<QString, QString> lastBlurTimestamps;

    void switchStreamForAllPlayers(const QString &suffix);

    CameraListDialog *cameraListDialog = nullptr;
    QNetworkAccessManager *networkManager;

    QMap<QString, QWebSocket*> socketMap;  // IP → QWebSocket*

    QGraphicsView *onvifView;
    QGraphicsScene *onvifScene;
    QGraphicsVideoItem *onvifVideoItem;
};

#endif // MAINWINDOW_H

