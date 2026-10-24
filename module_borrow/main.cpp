#include "borrowpage.h"
#include "databasemanager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QString dbPath = QCoreApplication::applicationDirPath()
		+ "/mytestborrow.db";//独立测试数据库

	//初始化数据库
	if (!DatabaseManager::instance().open(dbPath))
	{
		QMessageBox::critical(
			nullptr,
			QString::fromUtf8("数据库错误"),
			QString::fromUtf8("数据库初始化失败")
		);
		return 1;
	}

	//初始化成功后显示borrow界面
	BorrowPage w;
	w.show();

	return a.exec();
}