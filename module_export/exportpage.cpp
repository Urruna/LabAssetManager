#include "exportpage.h"
#include "ui_exportpage.h"
#include "csvexporter.h"
#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQueryModel>
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
               m.type AS '类型', m.cost AS '费用', m.description AS '描述'
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

    int availableCount = 0, borrowedCount = 0, repairingCount = 0, scrappedCount = 0;

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();

        if (status == "available") availableCount = count;
        else if (status == "borrowed") borrowedCount = count;
        else if (status == "repairing") repairingCount = count;
        else if (status == "scrapped") scrappedCount = count;
    }

    // 更新 UI 上的卡片标签（对象名稍后在 UI 里设置）
    ui->label_available->setText(QString("在用设备：%1 台").arg(availableCount));
    ui->label_borrowed->setText(QString("借出设备：%1 台").arg(borrowedCount));
    ui->label_repairing->setText(QString("维修中：%1 台").arg(repairingCount));
    ui->label_scrapped->setText(QString("已报废：%1 台").arg(scrappedCount));
}