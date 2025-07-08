#ifndef CAMERAREGISTRATIONDIALOG_H
#define CAMERAREGISTRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class CameraRegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraRegistrationDialog(QWidget *parent = nullptr);

    QString getCameraName() const;
    QString getCameraIP() const;
    QString getCameraPort() const;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    void setupUI();

    QLineEdit *nameEdit;
    QLineEdit *ipEdit;
    QLineEdit *portEdit;

    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // CAMERAREGISTRATIONDIALOG_H
