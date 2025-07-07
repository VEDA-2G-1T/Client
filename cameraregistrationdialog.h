#ifndef CAMERAREGISTRATIONDIALOG_H
#define CAMERAREGISTRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

class CameraRegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraRegistrationDialog(QWidget *parent = nullptr);
    QString getCameraInfo() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();

    QLineEdit *cameraNameEdit;
    QLineEdit *cameraIpEdit;
    QLineEdit *cameraPortEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QString cameraInfo;
};

#endif // CAMERAREGISTRATIONDIALOG_H
