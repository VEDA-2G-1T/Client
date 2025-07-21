#include "loginwindow.h"
#include "mainwindow.h"
#include <QApplication>
#include <QScreen>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent), mainWindow(nullptr)
{
    setupUI();
    setWindowTitle("Login - QtClientSSN Camera Monitoring System");

    // 창 크기 자동 조절
    adjustSize();
    setMinimumSize(sizeHint());

    // 중앙 정렬 (Qt6 기준)
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    move(screenGeometry.center() - rect().center());
}

LoginWindow::~LoginWindow()
{
    if (mainWindow) {
        delete mainWindow;
    }
}

void LoginWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    QLabel *titleLabel = new QLabel("Smart SafetyNet");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin: 20px;");

    // Username
    QLabel *usernameLabel = new QLabel("Username:");
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("Enter username");

    // Password
    QLabel *passwordLabel = new QLabel("Password:");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Enter password");

    // Login button
    loginButton = new QPushButton("Login");
    loginButton->setMinimumHeight(30);

    // 종료 버튼 추가
    QPushButton *exitButton = new QPushButton("Exit");
    exitButton->setMinimumHeight(30);

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(exitButton);

    // Add widgets
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(usernameEdit);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    // Connect signals
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(exitButton, &QPushButton::clicked, this, &LoginWindow::close);

    // Apply dark theme
    setStyleSheet(R"(
        QWidget {
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
            padding: 8px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username == "admin" && password == "admin") {
        mainWindow = new MainWindow();
        mainWindow->show();
        qDebug() << "MainWindow opened.";
        this->hide();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password!");
    }
}
