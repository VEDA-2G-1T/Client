#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QTableWidget>
#include <QMediaPlayer>
#include <QVideoWidget>

class CameraListDialog;

struct CameraInfo {
    QString name;
    QString ip;
    QString port;
    QString streamId;

    QString rtspUrl() const {
        return QString("rtsp://%1:%2/%3").arg(ip, port, streamId);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void refreshVideoGrid();  // 외부에서 카메라 갱신 시 호출

private slots:
    void onCameraListClicked();
    void onLogHistoryClicked();
    void onPPEDetectorToggled(bool enabled);
    void onMosaicerToggled(bool enabled);

private:
    void setupUI();
    void addLogEntry(const QString &camera, const QString &alert);

    // 📷 카메라 관련
    QVector<CameraInfo> cameraList;
    QVector<QMediaPlayer*> players;
    QVector<QVideoWidget*> videoWidgets;

    // 🖥️ UI 요소
    QWidget* centralWidget;
    QWidget* videoArea;                // ✅ 영상 그리드 컨테이너
    QGridLayout* videoGridLayout;
    QScrollArea* scrollArea;
    QTableWidget* logTable;

    QPushButton* cameraListButton;
    QCheckBox* ppeDetectorCheckBox;
    QCheckBox* mosaicerCheckBox;

    // 기타
    int currentCameraNumber;
    CameraListDialog* cameraListDialog = nullptr;
};

#endif // MAINWINDOW_H
