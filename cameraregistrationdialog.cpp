#include "cameraregistrationdialog.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

CameraRegistrationDialog::CameraRegistrationDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Camera Registration");
    setFixedSize(300, 200);
    setModal(true);
}

void CameraRegistrationDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Camera name
    QLabel *nameLabel = new QLabel("Camera Name:");
    cameraNameEdit = new QLineEdit();
    cameraNameEdit->setPlaceholderText("e.g., Front Door Camera");

    // Camera IP
    QLabel *ipLabel = new QLabel("Camera IP:");
    cameraIpEdit = new QLineEdit();
    cameraIpEdit->setPlaceholderText("e.g., 192.168.1.100");

    // IP validation (정규표현식)
    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    cameraIpEdit->setValidator(ipValidator);

    // Camera Port (optional)
    QLabel *portLabel = new QLabel("Port (optional):");
    cameraPortEdit = new QLineEdit();
    cameraPortEdit->setPlaceholderText("e.g., 8080");

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");

    // ✅ 버튼 최소 너비 설정
    okButton->setMinimumWidth(80);
    cancelButton->setMinimumWidth(80);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    // Layout 구성
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(cameraNameEdit);
    mainLayout->addWidget(ipLabel);
    mainLayout->addWidget(cameraIpEdit);
    mainLayout->addWidget(portLabel);
    mainLayout->addWidget(cameraPortEdit);
    mainLayout->addLayout(buttonLayout);

    // Signal 연결
    connect(okButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onCancelClicked);

    // 다크 테마 스타일 적용 (min-width 제거됨)
    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: white;
        }
        QLabel {
            color: white;
        }
        QLineEdit {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 5px;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 8px 16px;  // ✅ 글자 여유 확보
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}


QString CameraRegistrationDialog::getCameraInfo() const
{
    return cameraInfo;
}

void CameraRegistrationDialog::onOkClicked()
{
    QString name = cameraNameEdit->text().trimmed();
    QString ip = cameraIpEdit->text().trimmed();
    QString port = cameraPortEdit->text().trimmed();

    if (name.isEmpty() || ip.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter both camera name and IP address.");
        return;
    }

    // Create camera info string
    if (port.isEmpty()) {
        cameraInfo = QString("%1 (%2)").arg(name, ip);
    } else {
        cameraInfo = QString("%1 (%2:%3)").arg(name, ip, port);
    }

    accept();
}

void CameraRegistrationDialog::onCancelClicked()
{
    reject();
}
