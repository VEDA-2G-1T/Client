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
    explicit LogHistoryDialog(QWidget *parent = nullptr);

private slots:
    void onCloseClicked();

private:
    void setupUI();
    void loadHistoryData();

    QTableWidget *historyTable;
    QPushButton *closeButton;
};

#endif // LOGHISTORYDIALOG_H
