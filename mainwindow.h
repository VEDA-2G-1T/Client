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
#include <QSet>  // ✅ 이 줄 추가!
#include <QWebSocket>
#include <QMap>

class CameraListDialog;

struct CameraInfo {
    QString name;
    QString ip;
    QString port;

    QString rtspUrl() const {
        return QString("rtsps://%1:%2/raw").arg(ip, port);
    }

    bool operator==(const CameraInfo &other) const {
        return name == other.name && ip == other.ip && port == other.port;
    }
};

// logentry.h 또는 mainwindow.h 내부 등 구조체 선언부에 아래처럼 추가
struct LogEntry {
    QString camera;
    QString function;   // 👈 명시적 필드 추가
    QString alert;
    QString imagePath;
    QString details;
    QString date;
    QString time;
    int zone;  // ✅ 실제 스트리밍 영역 번호
    QString ip; // ✅ IP 필드 추가

};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refreshVideoGrid();
    QSet<QString> recentBlurLogKeys;  // ✅ 중복 Blur 로그 방지용 키



private slots:
    void onCameraListClicked();
    void onLogHistoryClicked();
    void sendModeChangeRequest(const QString &mode, const CameraInfo &camera);
    void onAlertItemClicked(int row, int column);  // ✅ 새로 추가

private:
    void setupUI();
    void setupTopBar();
    void setupVideoSection();
    void setupLogSection();
    void setupFunctionPanel();
    void setupMainLayout();
    /*
    void addLogEntry(const CameraInfo &camera, const QString &event, const QString &imagePath, const QString &details);
    void addLogEntry(const QString &cameraName, const QString &event,
                     const QString &imagePath, const QString &details, const QString &ip);
    void addLogEntry(const CameraInfo &camera, const QString &function, const QString &event, const QString &imagePath, const QString &details);  // ✅ 새 시그니처
    */
    void addLogEntry(const QString &cameraName,
                     const QString &function,
                     const QString &event,
                     const QString &imagePath,
                     const QString &details,
                     const QString &ip);
    void loadInitialLogs();  // ← private: 섹션에 선언

    QHBoxLayout *topLayout;
    QWidget *videoSection;
    QWidget *logSection;
    QWidget *functionSection;

    void setupWebSocketConnections();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketMessageReceived(const QString &message);
    void onSocketErrorOccurred(QAbstractSocket::SocketError error);

    QVector<CameraInfo> cameraList;
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;
    QVector<LogEntry> fullLogEntries;
    QMap<QString, QString> lastAnomalyStatus;  // 카메라 이름 -> 마지막 상태 ("detected"/"cleared")

    QWidget *centralWidget;
    QWidget *videoArea;
    QGridLayout *videoGridLayout;
    QScrollArea *scrollArea;
    QTableWidget *logTable;

    QPushButton *cameraListButton;

    QCheckBox *rawCheckBox;
    QCheckBox *blurCheckBox;
    QCheckBox *ppeDetectorCheckBox;

    QMap<QString, QString> lastPpeTimestamps;
    QMap<QString, QString> lastBlurTimestamps;

    void switchStreamForAllPlayers(const QString &suffix);

    CameraListDialog *cameraListDialog = nullptr;
    QNetworkAccessManager *networkManager;

    QMap<QString, QWebSocket*> socketMap;  // IP → QWebSocket*
};

#endif // MAINWINDOW_H

