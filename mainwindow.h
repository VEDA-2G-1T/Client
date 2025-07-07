#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QCheckBox>
#include <QMediaPlayer>
#include <QVideoWidget>

class CameraRegistrationDialog;
class LogHistoryDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCameraRegistrationClicked();
    void onCameraSelectChanged(const QString &cameraName);
    void onLogHistoryClicked();
    void onMosaicerToggled(bool enabled);
    void onPPEDetectorToggled(bool enabled);

private:
    void setupUI();
    void setupVideoStreamingSection();
    void setupLogSection();
    void addLogEntry(const QString &camera, const QString &alert);

    // UI Layouts
    QWidget *centralWidget;
    QVBoxLayout *outerLayout;        // ✅ 추가
    QHBoxLayout *mainLayout;

    // Video Streaming Section
    QWidget *videoSection;
    QLabel *videoStreamingLabel;
    QLabel *cameraNumberLabel;
    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;

    // Log Section
    QWidget *logSection;
    QLabel *logLabel;
    QPushButton *logHistoryButton;
    QTableWidget *logTable;

    // Controls (Bottom)
    QPushButton *cameraRegistrationButton;
    QComboBox *cameraSelectCombo;
    QCheckBox *mosaicerCheckBox;
    QCheckBox *ppeDetectorCheckBox;

    // Data
    QStringList registeredCameras;
    int currentCameraNumber;
};

#endif // MAINWINDOW_H
