#include <QAbstractItemView>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include "maintenancedialog.h"
#include "maintenancepage.h"

MaintenancePage::MaintenancePage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refresh();
}

void MaintenancePage::setupUi()
{
    setObjectName("maintenancePage");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    QLabel* titleLabel = new QLabel("维修保养台账", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    rootLayout->addWidget(titleLabel);

    QHBoxLayout* toolLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel("设备：", this);
    m_equipmentFilter = new QComboBox(this);
    m_equipmentFilter->setMinimumWidth(240);
    m_addButton = new QPushButton("新增记录", this);
    m_editButton = new QPushButton("编辑记录", this);
    m_refreshButton = new QPushButton("刷新", this);
    m_editButton->setEnabled(false);

    toolLayout->addWidget(filterLabel);
    toolLayout->addWidget(m_equipmentFilter);
    toolLayout->addStretch();
    toolLayout->addWidget(m_addButton);
    toolLayout->addWidget(m_editButton);
    toolLayout->addWidget(m_refreshButton);
    rootLayout->addLayout(toolLayout);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "设备编号", "设备名称", "日期", "类型", "说明", "费用", "经办人"
    });
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    rootLayout->addWidget(m_table, 1);

    m_statusLabel = new QLabel(this);
    rootLayout->addWidget(m_statusLabel);

    connect(m_equipmentFilter,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int) {
            refresh();
        });
    connect(m_addButton, &QPushButton::clicked, this, &MaintenancePage::addRecord);
    connect(m_editButton, &QPushButton::clicked, this, &MaintenancePage::editRecord);
    connect(m_refreshButton, &QPushButton::clicked, this, &MaintenancePage::refresh);
    connect(m_table,
        &QTableWidget::itemSelectionChanged,
        this,
        [this]() {
            m_editButton->setEnabled(m_table->currentRow() >= 0);
        });
    connect(m_table,
        &QTableWidget::cellDoubleClicked,
        this,
        [this](int, int) {
            editRecord();
        });
}

void MaintenancePage::refreshEquipmentFilter()
{
    QSignalBlocker blocker(m_equipmentFilter);
    int previousId = m_equipmentFilter->currentData().toInt();
    m_equipmentFilter->clear();
    m_equipmentFilter->addItem("全部设备", -1);

    QList<MaintenanceEquipmentOption> options =
        m_repository.findEquipmentOptions(false);
    for (const MaintenanceEquipmentOption& option : options) {
        QString text = option.assetNo + " - " + option.name;
        m_equipmentFilter->addItem(text, option.id);
    }

    int previousIndex = m_equipmentFilter->findData(previousId);
    m_equipmentFilter->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);
}

void MaintenancePage::refresh()
{
    refreshEquipmentFilter();

    int equipmentId = m_equipmentFilter->currentData().toInt();
    m_records = m_repository.findLedger(equipmentId);

    if (!m_repository.lastError().isEmpty()) {
        m_statusLabel->setText("查询失败：" + m_repository.lastError());
    }
    else {
        m_statusLabel->setText(QString("共 %1 条记录").arg(m_records.size()));
    }

    populateTable();
}

void MaintenancePage::populateTable()
{
    m_table->setRowCount(m_records.size());

    for (int row = 0; row < m_records.size(); ++row) {
        const MaintenanceLedgerItem& item = m_records.at(row);
        QString typeText = item.record.type == "repair" ? "维修" : "保养";
        QStringList values = {
            item.assetNo,
            item.equipmentName,
            item.record.maintenanceDate,
            typeText,
            item.record.description,
            QString::number(item.record.cost, 'f', 2) + " 元",
            item.record.operatorName
        };

        for (int column = 0; column < values.size(); ++column) {
            QTableWidgetItem* cell = new QTableWidgetItem(values.at(column));
            if (column == 0 || column == 2 || column == 3 ||
                column == 5 || column == 6) {
                cell->setTextAlignment(Qt::AlignCenter);
            }
            m_table->setItem(row, column, cell);
        }
    }

    m_editButton->setEnabled(false);
}

void MaintenancePage::addRecord()
{
    MaintenanceDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    MaintenanceRecord record = dialog.record();
    if (!m_repository.add(record, dialog.equipmentStatus())) {
        QMessageBox::warning(this, "保存失败", m_repository.lastError());
        return;
    }

    refresh();
    emit dataChanged();
}

void MaintenancePage::editRecord()
{
    int row = m_table->currentRow();
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

    refresh();
    emit dataChanged();
}
