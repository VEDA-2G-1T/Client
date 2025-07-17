#ifndef CAMERALISTDIALOG_H
#define CAMERALISTDIALOG_H

#include "mainwindow.h"
#include "camerainfo.h"

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>

class CameraListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraListDialog(QWidget *parent, QVector<CameraInfo>* cameraList);
    void refreshTable();

signals:
    void cameraListUpdated();  // MainWindow에 반영할 때 사용

private slots:
    void onAddCamera();
    void onRemoveCamera();

private:
    void setupUI();

    QVector<CameraInfo>* cameraListRef;
    QTableWidget* table;
    QPushButton* addButton;
    QPushButton* removeButton;
};

#endif // CAMERALISTDIALOG_H
