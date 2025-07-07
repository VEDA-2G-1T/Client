#include "loghistorydialog.h"
#include <QDateTime>

LogHistoryDialog::LogHistoryDialog(QWidget *parent)
    : QDialog(parent)
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

    // ✅ 행 번호 헤더 숨기기
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

    // Connect signals
    connect(closeButton, &QPushButton::clicked, this, &LogHistoryDialog::onCloseClicked);

    // Apply dark theme
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
            outline: none;     // ✅ 하얀 테두리 제거
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
    // Sample history data
    QStringList sampleEvents = {
        "System started",
        "Camera 1 connected",
        "Motion detected",
        "PPE violation detected",
        "Mosaicer enabled",
        "Camera 2 connected",
        "Normal operation resumed",
        "PPE Detector disabled",
        "Camera switched to Camera 2",
        "Alert cleared",
        "System maintenance",
        "Camera 1 reconnected"
    };

    QStringList cameras = {"System", "Camera 1", "Camera 2", "Camera 1", "System", "Camera 2", "Camera 1", "System", "System", "Camera 2", "System", "Camera 1"};

    QDateTime currentTime = QDateTime::currentDateTime();

    for (int i = 0; i < sampleEvents.size(); ++i) {
        int row = historyTable->rowCount();
        historyTable->insertRow(row);

        // Timestamp (going backwards in time)
        QDateTime eventTime = currentTime.addSecs(-i * 300); // 5 minutes apart
        historyTable->setItem(row, 0, new QTableWidgetItem(eventTime.toString("yyyy-MM-dd hh:mm:ss")));

        // Camera
        historyTable->setItem(row, 1, new QTableWidgetItem(cameras[i]));

        // Event
        historyTable->setItem(row, 2, new QTableWidgetItem(sampleEvents[i]));
    }

    // Resize columns to content
    historyTable->resizeColumnsToContents();
}

void LogHistoryDialog::onCloseClicked()
{
    accept();
}
