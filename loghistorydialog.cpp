#include "loghistorydialog.h"

#include <QDateTime>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

LogHistoryDialog::LogHistoryDialog(QWidget *parent, const QVector<LogEntry>* fullLogs)
    : QDialog(parent), logListPtr(fullLogs)
{
    setupUI();
    loadHistoryData();
    setWindowTitle("Safety Alerts History");
    setMinimumSize(600, 400);
    setModal(true);
}

void LogHistoryDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Complete Safety Alerts");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff8c00; margin-bottom: 10px;");

    historyTable = new QTableWidget(0, 3);
    historyTable->setHorizontalHeaderLabels(QStringList() << "Timestamp" << "Camera" << "Event");
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setAlternatingRowColors(true);
    historyTable->verticalHeader()->setVisible(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    closeButton = new QPushButton("Close");
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(historyTable);
    mainLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &LogHistoryDialog::onCloseClicked);

    connect(historyTable, &QTableWidget::cellClicked, this, &LogHistoryDialog::onRowClicked);

    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: white;
        }
        QLabel {
            color: white;
        }
        QTableWidget {
            background-color: #404040;
            color: white;
            gridline-color: #555;
            border: 1px solid #555;
            alternate-background-color: #353535;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QTableWidget::item:focus {
            outline: none;
            border: none;
        }
        QHeaderView::section {
            background-color: #353535;
            color: white;
            border: 1px solid #555;
            padding: 8px;
            font-weight: bold;
        }
        QPushButton {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 8px;
            border-radius: 4px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

void LogHistoryDialog::loadHistoryData()
{
    if (!logListPtr) return;

    for (int i = 0; i < logListPtr->size(); ++i) {
        const LogEntry& entry = logListPtr->at(i);

        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        QString timestamp = "Unknown";
        if (entry.alert.length() >= 19 && entry.alert[4].isDigit())
            timestamp = entry.alert.left(19);

        historyTable->setItem(row, 0, new QTableWidgetItem(timestamp));
        historyTable->setItem(row, 1, new QTableWidgetItem(entry.camera));
        historyTable->setItem(row, 2, new QTableWidgetItem(entry.alert));
    }

    historyTable->resizeColumnsToContents();
}

void LogHistoryDialog::onCloseClicked()
{
    accept();
}

void LogHistoryDialog::onRowClicked(int row, int column)
{
    if (!logListPtr || row >= logListPtr->size()) return;

    const LogEntry &entry = logListPtr->at(row);
    if (entry.imagePath.isEmpty()) {
        QMessageBox::information(this, "이미지 없음", "이 항목에는 이미지가 없습니다.");
        return;
    }

    // IP 주소 추출
    QString ip;
    if (entry.camera.contains("(") && entry.camera.contains(")")) {
        int start = entry.camera.indexOf('(') + 1;
        int end = entry.camera.indexOf(')');
        ip = entry.camera.mid(start, end - start);  // "Camera 1 (192.168.0.54)" → 192.168.0.54
    } else {
        ip = "192.168.0.54";  // 기본값
    }

    // 이미지 요청 URL 생성
    QString urlStr = QString("http://%1/%2").arg(ip, entry.imagePath);
    QUrl url(urlStr);  // ✅ 이렇게 따로 분리해서 생성
    QNetworkRequest request(url);

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "이미지 로딩 실패", reply->errorString());
            return;
        }

        QPixmap pix;
        pix.loadFromData(reply->readAll());
        if (pix.isNull()) {
            QMessageBox::warning(this, "이미지 오류", "유효한 이미지가 아닙니다.");
            return;
        }

        // 이미지 표시 다이얼로그 생성
        QDialog *imgDialog = new QDialog(this);
        imgDialog->setWindowTitle("감지 이미지");

        QLabel *imgLabel = new QLabel();
        imgLabel->setPixmap(pix.scaled(600, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QVBoxLayout *layout = new QVBoxLayout(imgDialog);
        layout->addWidget(imgLabel);
        imgDialog->setLayout(layout);
        imgDialog->setMinimumSize(640, 480);
        imgDialog->exec();
    });
}
