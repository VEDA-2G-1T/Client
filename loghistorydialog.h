#ifndef LOGHISTORYDIALOG_H
#define LOGHISTORYDIALOG_H

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
    explicit LogHistoryDialog(QWidget *parent = nullptr, const QVector<QPair<QString, QString>>* fullLogs = nullptr);

private slots:
    void onCloseClicked();

private:
    void setupUI();
    void loadHistoryData();

    const QVector<QPair<QString, QString>>* logListPtr = nullptr;  // ✅ 전체 로그 목록
    QTableWidget *historyTable;
    QPushButton *closeButton;
};

#endif // LOGHISTORYDIALOG_H
