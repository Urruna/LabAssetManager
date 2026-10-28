#pragma once

#include <QDialog>
#include <QDateTime>
#include "ui_returndialog.h"
#include "borrow_global.h"

class BORROW_EXPORT ReturnDialog : public QDialog
{
	Q_OBJECT

public:
	ReturnDialog(int recordId, QWidget *parent = nullptr);
	~ReturnDialog();
	QDateTime actualreturnTime() const;
	int returnedEquipmentId() const;
	bool isDamaged() const;

public slots:
	void accept() override;

private:
	Ui::ReturnDialogClass ui;
	int borrowRecordId;
	int equipmentId;
	QDateTime borrowTime;
};
