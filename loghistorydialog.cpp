#include "loghistorydialog.h"
#include <QDateTime>
#include <QTableWidgetItem>

LogHistoryDialog::LogHistoryDialog(QWidget *parent, const QVector<QPair<QString, QString>>* fullLogs)
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

    // Title
    QLabel *titleLabel = new QLabel("Complete Safety Alerts");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff8c00; margin-bottom: 10px;");

    // History table
    historyTable = new QTableWidget(0, 3);
    historyTable->setHorizontalHeaderLabels(QStringList() << "Timestamp" << "Camera" << "Event");
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setAlternatingRowColors(true);
    historyTable->verticalHeader()->setVisible(false);

    // Close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    closeButton = new QPushButton("Close");
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    // Main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(historyTable);
    mainLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &LogHistoryDialog::onCloseClicked);

    // Dark theme
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
        const QString& camera = logListPtr->at(i).first;
        const QString& alert = logListPtr->at(i).second;

        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        // ✅ Timestamp가 alert 문자열 안에 있는 경우 추출
        // 예: "2025-07-08 15:14:17 👷 1명 | ..." → 앞의 19자
        QString timestamp = "Unknown";
        if (alert.length() >= 19 && alert[4].isDigit())
            timestamp = alert.left(19);

        historyTable->setItem(row, 0, new QTableWidgetItem(timestamp));
        historyTable->setItem(row, 1, new QTableWidgetItem(camera));
        historyTable->setItem(row, 2, new QTableWidgetItem(alert));
    }

    historyTable->resizeColumnsToContents();
}

void LogHistoryDialog::onCloseClicked()
{
    accept();
}
