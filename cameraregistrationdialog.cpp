#include "cameraregistrationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

CameraRegistrationDialog::CameraRegistrationDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("카메라 등록");
    setFixedSize(360, 250);
    setModal(true);

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
            padding: 1px;
            border-radius: 4px;
            font-size: 13px;
        }
        QPushButton {
            font-size: 10px;
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

void CameraRegistrationDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 카메라 이름
    QLabel *nameLabel = new QLabel("카메라 이름:");
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("예: 작업장 #");

    // 카메라 IP
    QLabel *ipLabel = new QLabel("카메라 IP:");
    ipEdit = new QLineEdit();
    ipEdit->setPlaceholderText("예: 192.168.0.66");

    // 정규식 유효성 검사기
    QRegularExpression ipRegex("^(?:(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)$");
    QValidator *ipValidator = new QRegularExpressionValidator(ipRegex, this);
    ipEdit->setValidator(ipValidator);

    // 포트 번호
    QLabel *portLabel = new QLabel("포트번호:");
    portEdit = new QLineEdit();
    portEdit->setPlaceholderText("예: 8554");

    // 버튼 생성
    okButton = new QPushButton("등록");
    cancelButton = new QPushButton("취소");

    // ✅ 버튼 글자 안 잘리게 폭 고정
    okButton->setFixedHeight(30);
    cancelButton->setFixedHeight(30);

    // ✅ 버튼 폰트/패딩 스타일

    QString buttonStyle = R"(
        QPushButton {
            font-size: 12px;
        }
    )";
    okButton->setStyleSheet(buttonStyle);
    cancelButton->setStyleSheet(buttonStyle);

    // 버튼 레이아웃
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okButton);
    btnLayout->addWidget(cancelButton);

    // 메인 레이아웃 구성
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameEdit);
    mainLayout->addWidget(ipLabel);
    mainLayout->addWidget(ipEdit);
    mainLayout->addWidget(portLabel);
    mainLayout->addWidget(portEdit);
    mainLayout->addLayout(btnLayout);

    // ✅ 다이얼로그 크기 확장 (진짜 중요!)
    setFixedSize(420, 300);
    setModal(true);
    setWindowTitle("카메라 등록");

    // ✅ 스타일 적용
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
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");

    connect(okButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &CameraRegistrationDialog::onCancelClicked);
}


void CameraRegistrationDialog::onOkClicked()
{
    if (nameEdit->text().trimmed().isEmpty() ||
        ipEdit->text().trimmed().isEmpty() ||
        portEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "입력 오류", "모든 항목을 입력해주세요.");
        return;
    }

    accept();
}

void CameraRegistrationDialog::onCancelClicked()
{
    reject();
}

// Getters
QString CameraRegistrationDialog::getCameraName() const { return nameEdit->text().trimmed(); }
QString CameraRegistrationDialog::getCameraIP() const { return ipEdit->text().trimmed(); }
QString CameraRegistrationDialog::getCameraPort() const { return portEdit->text().trimmed(); }
