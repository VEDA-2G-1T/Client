#ifndef LOGHISTORYDIALOG_H
#define LOGHISTORYDIALOG_H

#include "mainwindow.h"  // ✅ LogEntry 구조체 정의 포함
#include "camerainfo.h"

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>

class LogHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogHistoryDialog(QWidget *parent = nullptr, const QVector<LogEntry>* fullLogs = nullptr);  // ✅ LogEntry로 변경

private slots:
    void onCloseClicked();
    void onRowClicked(int row, int column);  // ✅ 추가

private:
    void setupUI();
    void loadHistoryData();

    const QVector<LogEntry>* logListPtr = nullptr;  // ✅ QPair → LogEntry로 변경
    QTableWidget *historyTable;
    QPushButton *closeButton;
};

#endif // LOGHISTORYDIALOG_H
