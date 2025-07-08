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

private:
    void setupUI();
    void addLogEntry(const QString &camera, const QString &alert);

    QVector<CameraInfo> cameraList;
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;

    QWidget *centralWidget;
    QWidget *videoArea;
    QGridLayout *videoGridLayout;
    QScrollArea *scrollArea;
    QTableWidget *logTable;

    QPushButton *cameraListButton;

    QCheckBox *rawCheckBox;
    QCheckBox *blurCheckBox;
    QCheckBox *ppeDetectorCheckBox;

    void switchStreamForAllPlayers(const QString &suffix);

    CameraListDialog *cameraListDialog = nullptr;
    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H

