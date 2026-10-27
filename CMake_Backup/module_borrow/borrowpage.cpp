#include "csvexporter.h"
#include "borrowpage.h"
#include "ui_borrowpage.h"
#include <QDebug>
#include <QStandardItemModel>//表格数据
#include <QHeaderView>//表头设置
#include <QStringList>//文字列表
#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardItem>
#include <QMessageBox>
#include <QAbstractItemView>
#include "borrowdialog.h"
#include "returndialog.h"
#include <QDate>
#include <QFileDialog>

void BorrowPage::on_btn_export_borrow_clicked()
{
	// 1. 获取当前表格绑定的模型（如果你用了检索过滤代理模型，这里拿到的就是过滤后的数据！）
	QAbstractItemModel* currentModel = ui->tableView->model();

	if (currentModel->rowCount() == 0) {
		QMessageBox::information(this, "提示", "当前没有可导出的数据！");
		return;
	}

	// 2. 弹出保存对话框
	QString defaultFileName = QString("借还记录检索结果_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd"));
	QString filePath = QFileDialog::getSaveFileName(this, "导出数据", defaultFileName, "CSV Files (*.csv)");

	if (filePath.isEmpty()) return;

	// 3. 调用 D 同学写的导出工具
	if (CsvExporter::exportToCsv(currentModel, filePath)) {
		QMessageBox::information(this, "成功", "数据已成功导出！");
	}
	else {
		QMessageBox::critical(this, "失败", "导出失败，请检查文件是否被占用。");
	}
}

BorrowPage::BorrowPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BorrowPage)
{
    ui->setupUi(this);

    borrowModel = new QStandardItemModel(this);
    QStringList headerLabels;
    headerLabels << QString::fromUtf8("记录ID")
                 << QString::fromUtf8("资产编号")
                 << QString::fromUtf8("设备名称")
                 << QString::fromUtf8("借用人")
                 << QString::fromUtf8("借出时间")
                 << QString::fromUtf8("预计归还时间")
                 << QString::fromUtf8("实际归还时间")
                 << QString::fromUtf8("状态")
                 << QString::fromUtf8("备注");
    borrowModel->setHorizontalHeaderLabels(headerLabels);
    ui->tableView->setModel(borrowModel);
    ui->tableView->hideColumn(0);
    ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->date_from->setDate(QDate::currentDate().addMonths(-1));
	ui->date_to->setDate(QDate::currentDate());
	refreshData();
}

BorrowPage::~BorrowPage()
{
    delete ui;
}

void BorrowPage::on_btn_search_clicked()
{
	refreshData();
}

//刷新函数
void BorrowPage::refreshData()
{
	QString keyword = ui->edit_keyword->text().trimmed();
	int statusIndex = ui->combo_status->currentIndex();

	QDate fromDate = ui->date_from->date();
	QDate toDate = ui->date_to->date();
	if (fromDate > toDate)
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("日期错误"),
			QString::fromUtf8("开始日期不得晚于结束日期")
		);
		return;
	}

	QString sqlText =
		"SELECT b.id, e.asset_no, e.name, b.borrower, "
		"b.borrow_time, b.expected_return_time, "
		"b.actual_return_time, b.status, b.remark "
		"FROM borrow_record b "
		"JOIN equipment e ON b.equipment_id = e.id "
		"WHERE (b.borrower LIKE :borrowerKeyword "
		"OR e.name LIKE :equipmentKeyword) "
		"AND b.borrow_time >= :fromDate "
		"AND b.borrow_time < :toDate ";

	if (statusIndex == 1)
	{
		sqlText += "AND b.status = 'borrowed' ";
	}else if (statusIndex == 2)
	{
		sqlText += "AND b.status = 'returned' ";
	}

	sqlText += "ORDER BY b.borrow_time DESC, b.id DESC";

	QSqlQuery borrowQuery(DatabaseManager::instance().db());

	if (!borrowQuery.prepare(sqlText))
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("查询失败"),
			borrowQuery.lastError().text()
		);
		return;
	}

	borrowQuery.bindValue(":borrowerKeyword", "%" + keyword + "%");
	borrowQuery.bindValue(":equipmentKeyword", "%" + keyword + "%");
	borrowQuery.bindValue(":fromDate",fromDate.toString("yyyy-MM-dd") + " 00:00:00");
	borrowQuery.bindValue(":toDate",toDate.addDays(1).toString("yyyy-MM-dd") + " 00:00:00");

	if (!borrowQuery.exec())
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("查询失败"),
			borrowQuery.lastError().text()
		);
		return;
	}

	borrowModel->removeRows(0, borrowModel->rowCount());

	while (borrowQuery.next())
	{
		QList<QStandardItem *> rowItems;
		for (int col = 0; col < 9; ++col)
		{
			rowItems.append(
				new QStandardItem(borrowQuery.value(col).toString())
			);
		}
		if (borrowQuery.value(7).toString() == "borrowed")
		{
			rowItems[7]->setText(QString::fromUtf8("未归还"));
		}else if (borrowQuery.value(7).toString() == "returned")
		{
			rowItems[7]->setText(QString::fromUtf8("已归还"));
		}
		borrowModel->appendRow(rowItems);
	}

	borrowQuery.finish();

	ui->label_count->setText(
		QString::fromUtf8("共%1条记录").arg(borrowModel->rowCount())
	);
}

void BorrowPage::on_btn_borrow_clicked()
{
	BorrowDialog borrowDialog(this);
	if (borrowDialog.exec() == QDialog::Accepted)
	{
		refreshData();
		emit dataChanged();
		QMessageBox::information(
			this,
			QString::fromUtf8("成功"),
			QString::fromUtf8("借出登记完成")
		);
	}
}

void BorrowPage::on_btn_return_clicked()
{
	if (!ui->tableView->currentIndex().isValid())
	{
		QMessageBox::information(
			this,
			QString::fromUtf8("提示"),
			QString::fromUtf8("请先选择一条未归还记录")
		);
		return;
	}

	int recordId = borrowModel->index(ui->tableView->currentIndex().row(), 0).data().toInt();

	if (borrowModel->index(ui->tableView->currentIndex().row(), 7).data().toString() != QString::fromUtf8("未归还"))
	{
		QMessageBox::information(
			this,
			QString::fromUtf8("提示"),
			QString::fromUtf8("这条记录已经归还")
		);
		return;
	}

	ReturnDialog returnDialog(recordId, this);

	if (returnDialog.exec() == QDialog::Accepted)
	{
		const bool damaged = returnDialog.isDamaged();
		const int returnedEquipmentId = returnDialog.returnedEquipmentId();
		refreshData();
		emit dataChanged();
		if (damaged) {
			emit damagedEquipmentReturned(returnedEquipmentId);
		}
		QMessageBox::information(
			this,
			QString::fromUtf8("成功"),
			QString::fromUtf8("归还登记完成")
		);
	}
}
