#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QFileDialog>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "csvexporter.h"
#include "maintenancedialog.h"
#include "maintenancepage.h"
#include "ui_maintenancepage.h"

MaintenancePage::MaintenancePage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::MaintenancePage)
{
    ui->setupUi(this);

    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        5,
        QHeaderView::Stretch);

    connect(ui->combo_equipment,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int) {
            refreshData();
        });
    connect(ui->btn_add, &QPushButton::clicked, this, &MaintenancePage::addRecord);
    connect(ui->btn_edit, &QPushButton::clicked, this, &MaintenancePage::editRecord);
    connect(ui->btn_start, &QPushButton::clicked, this, &MaintenancePage::startRecord);
    connect(ui->btn_complete, &QPushButton::clicked, this, &MaintenancePage::completeRecord);
    connect(ui->btn_refresh, &QPushButton::clicked, this, &MaintenancePage::refreshData);
    connect(ui->tableView,
        &QTableWidget::itemSelectionChanged,
        this,
        &MaintenancePage::updateActionButtons);
    connect(ui->tableView,
        &QTableWidget::currentCellChanged,
        this,
        [this](int, int, int, int) {
            updateActionButtons();
        });
    connect(ui->tableView,
        &QTableWidget::cellDoubleClicked,
        this,
        [this](int, int) {
            editRecord();
        });
    connect(ui->btn_export, &QPushButton::clicked, this, &MaintenancePage::exportData);

    refreshData();
}

MaintenancePage::~MaintenancePage()
{
    delete ui;
}

void MaintenancePage::refreshEquipmentFilter()
{
    QSignalBlocker blocker(ui->combo_equipment);
    const int previousId = ui->combo_equipment->currentData().toInt();
    ui->combo_equipment->clear();
    ui->combo_equipment->addItem("全部设备", -1);

    const QList<MaintenanceEquipmentOption> options =
        m_repository.findEquipmentOptions(false);
    for (const MaintenanceEquipmentOption& option : options) {
        const QString text = option.assetNo + " - " + option.name;
        ui->combo_equipment->addItem(text, option.id);
    }

    const int previousIndex = ui->combo_equipment->findData(previousId);
    ui->combo_equipment->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);
}

void MaintenancePage::refreshData()
{
    refreshEquipmentFilter();

    const int equipmentId = ui->combo_equipment->currentData().toInt();
    m_records = m_repository.findLedger(equipmentId);

    if (!m_repository.lastError().isEmpty()) {
        ui->label_status->setText("查询失败：" + m_repository.lastError());
    }
    else {
        ui->label_status->setText(
            QString("共 %1 条记录").arg(m_records.size()));
    }

    populateTable();
}

void MaintenancePage::populateTable()
{
    ui->tableView->setRowCount(m_records.size());

    for (int row = 0; row < m_records.size(); ++row) {
        const MaintenanceLedgerItem& item = m_records.at(row);
        const QString typeText =
            item.record.type == "repair" ? "维修" : "保养";
        QString statusText =
            LabAsset::maintenanceStatusToDisplay(item.record.status);
        if (item.record.status == LabAsset::MaintenanceStatus::Pending) {
            statusText = "待维修";
        }

        const QStringList values = {
            item.assetNo,
            item.equipmentName,
            item.record.maintenanceDate,
            typeText,
            statusText,
            item.record.description,
            QString::number(item.record.cost, 'f', 2) + " 元",
            item.record.operatorName
        };

        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem* cell =
                new QTableWidgetItem(values.at(column));
            if (column == 0 || column == 2 || column == 3 ||
                column == 4 || column == 6 || column == 7) {
                cell->setTextAlignment(Qt::AlignCenter);
            }
            ui->tableView->setItem(row, column, cell);
        }
    }

    updateActionButtons();
}

void MaintenancePage::updateActionButtons()
{
    const int row = ui->tableView->currentRow();
    const bool selected =
        row >= 0 &&
        row < m_records.size() &&
        ui->tableView->selectionModel()->hasSelection();

    ui->btn_edit->setEnabled(selected);
    ui->btn_start->setEnabled(
        selected &&
        m_records.at(row).record.status ==
            LabAsset::MaintenanceStatus::Pending);
    ui->btn_complete->setEnabled(
        selected &&
        m_records.at(row).record.status ==
            LabAsset::MaintenanceStatus::InProgress);
}

void MaintenancePage::addRecord()
{
    MaintenanceDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    MaintenanceRecord record = dialog.record();
    if (!m_repository.add(record)) {
        QMessageBox::warning(this, "保存失败", m_repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

void MaintenancePage::startRecord()
{
    const int row = ui->tableView->currentRow();
    if (row < 0 || row >= m_records.size()) {
        QMessageBox::information(this, "提示", "请先选择一条待维修记录。");
        return;
    }

    if (!m_repository.start(m_records.at(row).record.id)) {
        QMessageBox::warning(this, "开始失败", m_repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

void MaintenancePage::completeRecord()
{
    const int row = ui->tableView->currentRow();
    if (row < 0 || row >= m_records.size()) {
        QMessageBox::information(this, "提示", "请先选择一条处理中的记录。");
        return;
    }

    if (!m_repository.complete(m_records.at(row).record.id)) {
        QMessageBox::warning(this, "完成失败", m_repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

bool MaintenancePage::createPendingRepairForDamagedReturn(int equipmentId)
{
    if (!m_repository.createPendingRepairForDamagedReturn(equipmentId)) {
        QMessageBox::warning(
            this,
            "创建待维修任务失败",
            m_repository.lastError());
        return false;
    }

    refreshData();
    emit dataChanged();
    return true;
}

void MaintenancePage::editRecord()
{
    const int row = ui->tableView->currentRow();
    if (row < 0 || row >= m_records.size()) {
        QMessageBox::information(this, "提示", "请先选择一条记录。");
        return;
    }

    MaintenanceDialog dialog(m_records.at(row).record, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (!m_repository.update(dialog.record())) {
        QMessageBox::warning(this, "更新失败", m_repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

void MaintenancePage::exportData()
{
    if (m_records.isEmpty()) {
        QMessageBox::information(this, "提示", "当前表格没有可导出的数据！");
        return;
    }

    const QString defaultFileName =
        QString("维保记录检索结果_%1.csv")
            .arg(QDate::currentDate().toString("yyyyMMdd"));
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "导出数据",
        defaultFileName,
        "CSV Files (*.csv)");

    if (filePath.isEmpty()) {
        return;
    }

    if (CsvExporter::exportToCsv(ui->tableView->model(), filePath)) {
        QMessageBox::information(this, "成功", "数据已成功导出！");
    }
    else {
        QMessageBox::critical(
            this,
            "失败",
            "导出失败，请检查文件是否被占用。");
    }
}
