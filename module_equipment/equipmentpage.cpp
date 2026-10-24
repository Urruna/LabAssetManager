#include "equipmentpage.h"

#include "equipmentdialog.h"
#include "equipmentrepository.h"
#include "ui_equipmentpage.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableView>

#include <algorithm>

namespace {

class EquipmentSortProxyModel : public QSortFilterProxyModel
{
public:
    explicit EquipmentSortProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setSortCaseSensitivity(Qt::CaseInsensitive);
    }

protected:
    bool lessThan(
        const QModelIndex& left,
        const QModelIndex& right) const override
    {
        // 价格列使用数值排序，不按格式化后的字符串排序。
        if (left.column() == 5 && right.column() == 5) {
            return left.data(Qt::UserRole).toDouble()
                < right.data(Qt::UserRole).toDouble();
        }
        return QSortFilterProxyModel::lessThan(left, right);
    }
};

} // namespace

EquipmentPage::EquipmentPage(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::EquipmentPage)
{
    ui->setupUi(this);
    setupTable();

    ui->combo_status->addItem("全部状态", QString());
    for (const QString& status : equipmentStatusValues()) {
        ui->combo_status->addItem(equipmentStatusText(status), status);
    }

    connect(
        ui->btn_search,
        &QPushButton::clicked,
        this,
        &EquipmentPage::refreshData);
    connect(
        ui->edit_keyword,
        &QLineEdit::returnPressed,
        this,
        &EquipmentPage::refreshData);
    connect(
        ui->combo_status,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int) {
            refreshData();
        });
    connect(
        ui->btn_add,
        &QPushButton::clicked,
        this,
        &EquipmentPage::addEquipment);
    connect(
        ui->btn_edit,
        &QPushButton::clicked,
        this,
        &EquipmentPage::editEquipment);
    connect(
        ui->btn_delete,
        &QPushButton::clicked,
        this,
        &EquipmentPage::deleteEquipment);
    connect(
        ui->btn_refresh,
        &QPushButton::clicked,
        this,
        &EquipmentPage::refreshData);
    connect(
        ui->tableView,
        &QTableView::doubleClicked,
        this,
        [this](const QModelIndex&) {
            editEquipment();
        });
    connect(
        ui->tableView->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        [this]() {
            const bool hasSelection = selectedEquipmentId() > 0;
            ui->btn_edit->setEnabled(hasSelection);
            ui->btn_delete->setEnabled(hasSelection);
        });

    refreshData();
}

EquipmentPage::~EquipmentPage()
{
    delete ui;
}

void EquipmentPage::setupTable()
{
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({
        "资产编号",
        "设备名称",
        "型号",
        "品牌",
        "购置日期",
        "价格（元）",
        "存放位置",
        "状态",
        "责任人",
        "备注"
    });

    m_proxy = new EquipmentSortProxyModel(this);
    m_proxy->setSourceModel(m_model);

    ui->tableView->setModel(m_proxy);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(false);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(
        1,
        QHeaderView::Stretch);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    ui->btn_edit->setEnabled(false);
    ui->btn_delete->setEnabled(false);
}

void EquipmentPage::refreshData()
{
    const QString keyword = ui->edit_keyword->text().trimmed();
    const QString status = ui->combo_status->currentData().toString();

    EquipmentRepository repository;
    m_equipment = repository.find(keyword, status);

    if (!repository.lastError().isEmpty()) {
        ui->label_status->setText(
            "查询失败：" + repository.lastError());
        m_equipment.clear();
    }
    else {
        ui->label_status->setText(
            QString("共找到 %1 台设备").arg(m_equipment.size()));
    }

    populateTable();
}

void EquipmentPage::populateTable()
{
    m_model->removeRows(0, m_model->rowCount());

    for (const Equipment& equipment : m_equipment) {
        QList<QStandardItem*> row;
        auto createItem = [](const QString& text) {
            return new QStandardItem(text);
        };

        QStandardItem* assetNoItem = createItem(equipment.assetNo);
        assetNoItem->setData(equipment.id, Qt::UserRole);
        row << assetNoItem;
        row << createItem(equipment.name);
        row << createItem(equipment.model);
        row << createItem(equipment.brand);
        row << createItem(equipment.purchaseDate);

        QStandardItem* priceItem =
            createItem(QString::number(equipment.price, 'f', 2));
        priceItem->setData(equipment.price, Qt::UserRole);
        row << priceItem;

        row << createItem(equipment.location);
        row << createItem(equipmentStatusText(equipment.status));
        row << createItem(equipment.keeper);
        row << createItem(equipment.remark);

        for (int column = 0; column < row.size(); ++column) {
            if (column == 0 || column == 4 || column == 5 || column == 7) {
                row.at(column)->setTextAlignment(Qt::AlignCenter);
            }
        }

        m_model->appendRow(row);
    }

    ui->tableView->clearSelection();
    ui->btn_edit->setEnabled(false);
    ui->btn_delete->setEnabled(false);
}

int EquipmentPage::selectedEquipmentId() const
{
    const QModelIndex proxyIndex = ui->tableView->currentIndex();
    if (!proxyIndex.isValid()) {
        return -1;
    }

    const QModelIndex sourceIndex = m_proxy->mapToSource(proxyIndex);
    QStandardItem* item = m_model->item(sourceIndex.row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

const Equipment* EquipmentPage::findEquipment(int id) const
{
    const auto it = std::find_if(
        m_equipment.cbegin(),
        m_equipment.cend(),
        [id](const Equipment& equipment) {
            return equipment.id == id;
        });
    return it == m_equipment.cend() ? nullptr : &(*it);
}

void EquipmentPage::addEquipment()
{
    EquipmentDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Equipment equipment = dialog.equipment();
    EquipmentRepository repository;
    if (!repository.add(equipment)) {
        QMessageBox::warning(this, "保存失败", repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

void EquipmentPage::editEquipment()
{
    const Equipment* current = findEquipment(selectedEquipmentId());
    if (!current) {
        QMessageBox::information(this, "提示", "请先选择一台设备。");
        return;
    }

    EquipmentDialog dialog(*current, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Equipment equipment = dialog.equipment();
    EquipmentRepository repository;
    if (!repository.update(equipment)) {
        QMessageBox::warning(this, "更新失败", repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}

void EquipmentPage::deleteEquipment()
{
    const Equipment* current = findEquipment(selectedEquipmentId());
    if (!current) {
        QMessageBox::information(this, "提示", "请先选择一台设备。");
        return;
    }

    const QString description =
        current->assetNo + " - " + current->name;
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "确认删除",
        QString("确定永久删除设备“%1”吗？\n此操作无法撤销。")
            .arg(description),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    EquipmentRepository repository;
    if (!repository.remove(current->id)) {
        QMessageBox::warning(this, "无法删除", repository.lastError());
        return;
    }

    refreshData();
    emit dataChanged();
}
