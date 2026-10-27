#include "mainwindow.h" // 引入主窗口
#include <QApplication>
#include "databasemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 1. 初始化数据库
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/lab_asset.db";

    if (!DatabaseManager::instance().open(dbPath)) {
        qCritical() << "数据库初始化失败";
        return -1;
    }

    // 2. 显示主窗口
    MainWindow w;
    w.show();

    // 3. 进入事件循环
    return a.exec();
}