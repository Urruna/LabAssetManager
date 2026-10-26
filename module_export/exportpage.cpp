#include "exportpage.h"
#include "ui_exportpage.h"
#include "csvexporter.h"
#include "databasemanager.h"
#include "enums.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QFont>
#include <QMap>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QDate>

// 这是一个内部辅助函数，用来处理保存对话框和导出
void ExportPage::doExport(const QString& defaultFileName, const QString& sqlQuery)
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出数据", defaultFileName, "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(sqlQuery, DatabaseManager::instance().db());

    if (CsvExporter::exportToCsv(model, filePath)) {
        QMessageBox::information(this, "成功", "导出成功！");
    }
    else {
        QMessageBox::critical(this, "失败", "文件保存失败！");
    }
    model->deleteLater();
}

// 按钮 1：导出设备总览
void ExportPage::on_btn_export_equipment_clicked()
{
    QString fileName = QString("设备总览_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd"));
    QString sql = "SELECT asset_no AS '资产编号', name AS '设备名称', model AS '型号', status AS '状态', location AS '位置' FROM equipment";
    doExport(fileName, sql);
}

// 按钮 2：导出借还记录
void ExportPage::on_btn_export_borrow_clicked()
{
    QString fileName = QString("借还记录_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd"));
    QString sql = R"(
        SELECT b.borrower AS '借用人', e.name AS '设备名称', 
               b.borrow_time AS '借出时间', b.actual_return_time AS '归还时间', 
               b.status AS '状态'
        FROM borrow_record b 
        LEFT JOIN equipment e ON b.equipment_id = e.id
    )";
    doExport(fileName, sql);
}

// 按钮 3：导出维修记录
void ExportPage::on_btn_export_maintenance_clicked()
{
    QString fileName = QString("维修记录_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd"));
    QString sql = R"(
        SELECT m.maintenance_date AS '日期', e.name AS '设备名称', 
               m.type AS '类型',
               CASE m.status
                   WHEN 'pending' THEN '待维修'
                   WHEN 'in_progress' THEN '处理中'
                   WHEN 'completed' THEN '已完成'
                   ELSE m.status
               END AS '任务状态',
               m.cost AS '费用', m.description AS '描述'
        FROM maintenance_record m 
        LEFT JOIN equipment e ON m.equipment_id = e.id
    )";
    doExport(fileName, sql);
}

ExportPage::ExportPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ExportPage)
{
    ui->setupUi(this);
    // 页面初始化时先刷新一次数据
    refreshData();
}

ExportPage::~ExportPage()
{
    delete ui;
}

void ExportPage::refreshData()
{
    QSqlQuery query(DatabaseManager::instance().db());
    query.exec("SELECT status, COUNT(*) FROM equipment GROUP BY status");

    QMap<int, int> statusCounts;
    for (LabAsset::EquipmentStatus status :
         LabAsset::allEquipmentStatuses()) {
        statusCounts.insert(static_cast<int>(status), 0);
    }

    int total = 0;

    while (query.next()) {
        const QString rawStatus = query.value(0).toString();
        const auto status = LabAsset::statusFromDb(rawStatus);
        if (!status) {
            continue;
        }

        const int count = query.value(1).toInt();
        total += count;
        statusCounts[static_cast<int>(*status)] = count;
    }

    auto countFor = [&statusCounts](LabAsset::EquipmentStatus status) {
        return statusCounts.value(static_cast<int>(status), 0);
    };

    auto calcPercent = [total](int count) -> QString {
        if (total == 0) return "0.0%";
        double p = (double)count / total * 100.0;
        return QString::number(p, 'f', 1) + "%";
        };

    // 更新 UI 上的卡片标签
    ui->label_available->setText(
        QString("在用设备：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::Available)));
    ui->label_borrowed->setText(
        QString("借出设备：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::Borrowed)));
    ui->label_pending_repair->setText(
        QString("待维修：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::PendingRepair)));
    ui->label_repairing->setText(
        QString("维修中：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::Repairing)));
    ui->label_under_maintenance->setText(
        QString("保养中：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::Maintaining)));
    ui->label_scrapped->setText(
        QString("已报废：%1 台")
            .arg(countFor(LabAsset::EquipmentStatus::Scrapped)));

    // 2. 更新中间的 tableView
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ "设备状态", "数量 (台)", "占比" });

    // 辅助插入行的 Lambda 函数
    auto addRow = [model](QString statusName, int count, QString percent) {
        QList<QStandardItem*> row;
        QStandardItem* itemStatus = new QStandardItem(statusName);
        QStandardItem* itemCount = new QStandardItem(QString::number(count));
        QStandardItem* itemPercent = new QStandardItem(percent);

        // 居中对齐
        itemStatus->setTextAlignment(Qt::AlignCenter);
        itemCount->setTextAlignment(Qt::AlignCenter);
        itemPercent->setTextAlignment(Qt::AlignCenter);

        row << itemStatus << itemCount << itemPercent;
        model->appendRow(row);
        };

    for (LabAsset::EquipmentStatus status :
         LabAsset::allEquipmentStatuses()) {
        const int count = countFor(status);
        addRow(
            LabAsset::statusToDisplay(status),
            count,
            calcPercent(count));
    }

    // 插入合计行
    QList<QStandardItem*> totalRow;
    QStandardItem* itemTotal = new QStandardItem("总计");
    QStandardItem* itemTotalCount = new QStandardItem(QString::number(total));
    QStandardItem* itemTotalPercent = new QStandardItem("100%");
    itemTotal->setTextAlignment(Qt::AlignCenter);
    itemTotalCount->setTextAlignment(Qt::AlignCenter);
    itemTotalPercent->setTextAlignment(Qt::AlignCenter);
    // 让合计行的字体加粗
    QFont boldFont;
    boldFont.setBold(true);
    itemTotal->setFont(boldFont);
    itemTotalCount->setFont(boldFont);
    itemTotalPercent->setFont(boldFont);
    totalRow << itemTotal << itemTotalCount << itemTotalPercent;
    model->appendRow(totalRow);

    // 把模型绑定到 UI 里的 tableView
    ui->tableView->setModel(model);

    // 美化表格：让表头自适应宽度，隐藏左侧行号
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
