#include "returndialog.h"
#include "databasemanager.h"
#include "enums.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QPushButton>

ReturnDialog::ReturnDialog(int recordId, QWidget *parent):QDialog(parent),borrowRecordId(recordId),equipmentId(0)
{
	ui.setupUi(this);
	ui.dt_actual_return_time->setDateTime(QDateTime::currentDateTime());

	connect(ui.buttonBox, &QDialogButtonBox::accepted,this, &ReturnDialog::accept);
	connect(ui.buttonBox, &QDialogButtonBox::rejected,this, &QDialog::reject);

	QSqlQuery returnQuery(DatabaseManager::instance().db());
	returnQuery.prepare(
		"SELECT b.equipment_id, b.borrow_time, "
		"e.asset_no, e.name, b.borrower "
		"FROM borrow_record b "
		"JOIN equipment e ON b.equipment_id = e.id "
		"WHERE b.id = :recordId "
		"AND b.status = :borrowedStatus "
		"AND b.actual_return_time IS NULL"
	);

	returnQuery.bindValue(":recordId", borrowRecordId);
	returnQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));

	if (!returnQuery.exec() || !returnQuery.next())
	{
		ui.label_info->setText(QString::fromUtf8("记录不存在或已经归还，请取消后刷新列表"));
		ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

		if (returnQuery.lastError().isValid())
		{
			QMessageBox::warning(
				this,
				QString::fromUtf8("读取失败"),
				returnQuery.lastError().text()
			);
		}
		return;
	}

	equipmentId = returnQuery.value(0).toInt();

	borrowTime = QDateTime::fromString(returnQuery.value(1).toString(),"yyyy-MM-dd HH:mm:ss");

	ui.label_info->setText(
		QString::fromUtf8(
			"资产编号：%1\n设备：%2\n借用人：%3\n"
			"借出时间：%4\n\n实际归还时间："
		)
		.arg(returnQuery.value(2).toString())
		.arg(returnQuery.value(3).toString())
		.arg(returnQuery.value(4).toString())
		.arg(returnQuery.value(1).toString())
	);

	returnQuery.finish();

	if (borrowTime.isValid())
	{
		ui.dt_actual_return_time->setMinimumDateTime(borrowTime);
	}
}

ReturnDialog::~ReturnDialog()
{
}

QDateTime ReturnDialog::actualreturnTime() const
{
	return ui.dt_actual_return_time->dateTime();
}

int ReturnDialog::returnedEquipmentId() const
{
	return equipmentId;
}

bool ReturnDialog::isDamaged() const
{
	return ui.check_damaged->isChecked();
}

void ReturnDialog::accept()
{
	if (equipmentId <= 0 || !borrowTime.isValid() || !actualreturnTime().isValid() || actualreturnTime() < borrowTime)
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("无法归还"),
			QString::fromUtf8("记录或时间无效，归还时间不得早于借出时间")
		);
		return;
	}

	QSqlDatabase db = DatabaseManager::instance().db();
	QString errorText;

	if (!db.transaction())
	{
		QMessageBox::warning(
			this,
			QString::fromUtf8("归还失败"),
			db.lastError().text()
		);
		return;
	}

	QSqlQuery returnQuery(db);

	returnQuery.prepare(
		"UPDATE borrow_record "
		"SET status = 'returned', "
		"actual_return_time = :actualreturnTime "
		"WHERE id = :recordId "
		"AND equipment_id = :equipmentId "
		"AND status = :borrowedStatus "
		"AND actual_return_time IS NULL "
		"AND borrow_time <= :actualreturnTime"
	);

	returnQuery.bindValue(":actualreturnTime",actualreturnTime().toString("yyyy-MM-dd HH:mm:ss"));

	returnQuery.bindValue(":recordId", borrowRecordId);
	returnQuery.bindValue(":equipmentId", equipmentId);
	returnQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));

	if (!returnQuery.exec() || returnQuery.numRowsAffected() != 1)
	{
		errorText = returnQuery.lastError().text();
		if (errorText.isEmpty())
		{
			errorText = QString::fromUtf8("记录已变化或已经归还，请刷新后重试");
		}
		returnQuery.finish();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("归还失败"), errorText);
		return;
	}

	returnQuery.finish();

	returnQuery.prepare(
		"UPDATE equipment SET status = :equipmentStatus "
		"WHERE id = :equipmentId AND status = :borrowedStatus "
		"AND NOT EXISTS ("
		"SELECT 1 FROM borrow_record b "
		"WHERE b.equipment_id = equipment.id "
		"AND (b.status = :borrowedStatus OR b.actual_return_time IS NULL)"
		")"
	);

	returnQuery.bindValue(
		":equipmentStatus",
		LabAsset::statusToDb(
			ui.check_damaged->isChecked()
				? LabAsset::EquipmentStatus::PendingRepair
				: LabAsset::EquipmentStatus::Available)
	);

	returnQuery.bindValue(":equipmentId", equipmentId);
	returnQuery.bindValue(
		":borrowedStatus",
		LabAsset::statusToDb(LabAsset::EquipmentStatus::Borrowed));

	if (!returnQuery.exec() || returnQuery.numRowsAffected() != 1)
	{
		errorText = returnQuery.lastError().text();
		if (errorText.isEmpty())
		{
			errorText = QString::fromUtf8("设备状态与借还记录不一致，请核对");
		}
		returnQuery.finish();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("归还失败"), errorText);
		return;
	}

	returnQuery.finish();

	if (!db.commit())
	{
		errorText = db.lastError().text();
		db.rollback();
		QMessageBox::warning(this, QString::fromUtf8("归还失败"), errorText);
		return;
	}

	QDialog::accept();
}
