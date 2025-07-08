#include "cameralistdialog.h"
#include "cameraregistrationdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QHeaderView>


CameraListDialog::CameraListDialog(QWidget *parent, QVector<CameraInfo>* cameraList)
    : QDialog(parent), cameraListRef(cameraList)
{
    setupUI();
    refreshTable();
    setWindowTitle("카메라 리스트");
    setMinimumSize(700, 400);
    setModal(true);

    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: white;
        }
        QTableWidget {
            background-color: #404040;
            color: white;
            gridline-color: #555;
        }
        QHeaderView::section {
            background-color: #353535;
            color: white;
            font-weight: bold;
        }
        QPushButton {
            background-color: #404040;
            color: white;
            border: 1px solid #555;
            padding: 6px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
    )");
}

void CameraListDialog::setupUI()
{
    table = new QTableWidget(0, 4);
    table->setHorizontalHeaderLabels(QStringList() << "스트리밍 영역" << "카메라 이름" << "카메라 IP" << "포트번호");
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    addButton = new QPushButton("카메라 등록");
    removeButton = new QPushButton("카메라 삭제");

    connect(addButton, &QPushButton::clicked, this, &CameraListDialog::onAddCamera);
    connect(removeButton, &QPushButton::clicked, this, &CameraListDialog::onRemoveCamera);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addWidget(addButton);
    topLayout->addWidget(removeButton);
    topLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);
}

void CameraListDialog::refreshTable()
{
    table->setRowCount(0);
    for (int i = 0; i < cameraListRef->size(); ++i) {
        const CameraInfo& cam = cameraListRef->at(i);
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1))); // 스트리밍 영역 번호
        table->setItem(i, 1, new QTableWidgetItem(cam.name));
        table->setItem(i, 2, new QTableWidgetItem(cam.ip));
        table->setItem(i, 3, new QTableWidgetItem(cam.port));
    }
}

void CameraListDialog::onAddCamera()
{
    CameraRegistrationDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        CameraInfo info;
        info.name = dialog.getCameraName();
        info.ip = dialog.getCameraIP();
        info.port = dialog.getCameraPort();

        cameraListRef->append(info);
        refreshTable();
        emit cameraListUpdated();  // MainWindow 쪽에서 refreshVideoGrid 호출
    }
}

void CameraListDialog::onRemoveCamera()
{
    int row = table->currentRow();
    if (row < 0 || row >= cameraListRef->size()) {
        QMessageBox::warning(this, "선택 오류", "삭제할 카메라를 선택하세요.");
        return;
    }

    cameraListRef->removeAt(row);
    refreshTable();
    emit cameraListUpdated();  // 삭제 후에도 갱신
}
