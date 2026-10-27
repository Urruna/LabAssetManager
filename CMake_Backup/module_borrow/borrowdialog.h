#pragma once

#include <QDialog>
#include <QDateTime>
#include <QString>
#include "ui_borrowdialog.h"

class BorrowDialog : public QDialog
{
	Q_OBJECT

public:
	BorrowDialog(QWidget *parent = nullptr);
	~BorrowDialog();

	int equipmentId() const;
	QString borrower() const;
	QDateTime borrowTime() const;
	QDateTime expectedreturnTime() const;
	QString remark() const;

public slots:
	void accept() override;

private:
	Ui::BorrowDialogClass ui;
};