#include "borrowdialog.h"
#include "databasemanager.h"
#include "enums.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QPushButton>


BorrowDialog::BorrowDialog(QWidget *parent):QDialog(parent)
{
	ui.setupUi(this);

	ui.dt_borrow_time->setDateTime(QDateTime::currentDateTime());
	ui.dt_expected_return_time->setDateTime(QDateTime::currentDateTime().addDays(7));

	connect(ui.buttonBox, &QDialogButtonBox::accepted,this,&BorrowDialog::accept);
	connect(ui.buttonBox, &QDialogButtonBox::rejected,this,&QDialog::reject);

	QSqlQuery equipmentQuery(DatabaseManager::instance().db());
	if (!equipmentQuery.prepare(
		"SELECT e.id, e.asset_no, e.name "
		"FROM equipment e "
		"WHERE e.status = :availableStatus "
		"AND NOT EXISTS ("
		"SELECT 1 FROM borrow_record b "
		"WHERE b.equipment_id = e.id "
		"AND (b.status = :borrowedStatus OR b.actual_return_time IS NULL)"
		") "
		"ORDER BY e.asset_no")) {
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		QMessageBox::warning(this,QString::fromUtf8("读取设备失败"),equipmentQuery.lastError().text());
		return;
	}

	equipmentQuery.bindValue(
		":availableStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));
	equipmentQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));

	if (!equipmentQuery.exec()) {
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		QMessageBox::warning(
			this,
			QString::fromUtf8("读取设备失败"),
			equipmentQuery.lastError().text());
		return;
	}

	while (equipmentQuery.next())
	{
		ui.combo_equipment->addItem(
			equipmentQuery.value(1).toString() + " - "
			+ equipmentQuery.value(2).toString(),
			equipmentQuery.value(0).toInt()
		);
	}

	equipmentQuery.finish();

	if (ui.combo_equipment->count() == 0){
		ui.combo_equipment->addItem(QString::fromUtf8("暂无可借设备"));
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

BorrowDialog::~BorrowDialog()
{

}

int BorrowDialog::equipmentId() const
{
	return ui.combo_equipment->currentData().toInt();
}

QString BorrowDialog::borrower() const
{
	return ui.edit_borrower->text().trimmed();
}

QDateTime BorrowDialog::borrowTime() const
{
	return ui.dt_borrow_time->dateTime();
}

QDateTime BorrowDialog::expectedreturnTime() const
{
	return ui.dt_expected_return_time->dateTime();
}

QString BorrowDialog::remark() const
{
	return ui.edit_remark->toPlainText();
}

void BorrowDialog::accept()
{
	if (equipmentId() <= 0 || borrower().isEmpty())
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("检查输入"),
			QString::fromUtf8("请选择设备并填写借用人")
		);
		return;
	}

	if (!borrowTime().isValid() || !expectedreturnTime().isValid() || expectedreturnTime() < borrowTime())
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("检查输入"),
			QString::fromUtf8("预计归还时间不得早于借出时间")
		);
		return;
	}

	QSqlDatabase db = DatabaseManager::instance().db();
	QString errorText;

	if (!db.transaction())
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("借出失败"),
			db.lastError().text()
		);
		return;
	}//设备状态和借还记录需要一起修改

	QSqlQuery borrowQuery(db);

	borrowQuery.prepare(
		"UPDATE equipment SET status = :borrowedStatus "
		"WHERE id = :equipmentId AND status = :availableStatus "
		"AND NOT EXISTS ("
		"SELECT 1 FROM borrow_record b "
		"WHERE b.equipment_id = equipment.id "
		"AND (b.status = :borrowedStatus OR b.actual_return_time IS NULL)"
		")"
	);

	borrowQuery.bindValue(":equipmentId", equipmentId());
	borrowQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));
	borrowQuery.bindValue(
		":availableStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));

	if (!borrowQuery.exec())
	{
		errorText = borrowQuery.lastError().text();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("借出失败"), errorText);
		return;
	}

	if (borrowQuery.numRowsAffected() != 1)
	{
		db.rollback();
		QMessageBox::warning(
			this,
			QString::fromUtf8("借出失败"),
			QString::fromUtf8("设备已不可借，请取消后重新打开")
		);
		return;
	}

	borrowQuery.finish();

	borrowQuery.prepare(
		"INSERT INTO borrow_record("
		"equipment_id, borrower, borrow_time, expected_return_time, "
		"actual_return_time, status, remark) "
		"VALUES("
		":equipmentId, :borrower, :borrowTime, :expectedreturnTime, "
		"NULL, :borrowedStatus, :remark)"
	);
	borrowQuery.bindValue(":equipmentId",equipmentId());
	borrowQuery.bindValue(":borrower",borrower());
	borrowQuery.bindValue(":borrowTime",borrowTime().toString("yyyy-MM-dd HH:mm:ss"));
	borrowQuery.bindValue(":expectedreturnTime",expectedreturnTime().toString("yyyy-MM-dd HH:mm:ss"));
	borrowQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));
	borrowQuery.bindValue(":remark", remark());

	if (!borrowQuery.exec())
	{
		errorText = borrowQuery.lastError().text();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("借出失败"), errorText);
		return;
	}

	borrowQuery.finish();

	if (!db.commit())
	{
		errorText = db.lastError().text();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("借出失败"), errorText);
		return;
	}

	QDialog::accept();
}
