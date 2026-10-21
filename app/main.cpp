#include "databasemanager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/lab_asset.db";
    qDebug() << "数据库路径:" << dbPath;

    if (!DatabaseManager::instance().open(dbPath)) {
        qCritical() << "数据库初始化失败";
        return -1;
    }
    qDebug() << "数据库打开并建表成功";

    QSqlQuery q(DatabaseManager::instance().db());
    q.exec("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
    while (q.next())
        qDebug() << "  表:" << q.value(0).toString();

    QSqlQuery ins(DatabaseManager::instance().db());
    ins.prepare("INSERT INTO equipment(asset_no, name, status) VALUES(?, ?, ?)");
    ins.addBindValue("TEST-001");
    ins.addBindValue(QString::fromUtf8("测试设备"));
    ins.addBindValue("available");
    if (!ins.exec())
        qWarning() << "插入失败:" << ins.lastError().text();
    else
        qDebug() << "插入成功，id =" << ins.lastInsertId();

    QSqlQuery sel(DatabaseManager::instance().db());
    sel.exec("SELECT id, asset_no, name, status FROM equipment");
    while (sel.next()) {
        qDebug() << "  读到:" << sel.value(0).toInt()
            << sel.value(1).toString()
            << sel.value(2).toString()
            << sel.value(3).toString();
    }

    DatabaseManager::instance().close();
    qDebug() << "验证结束";
    return 0;
}