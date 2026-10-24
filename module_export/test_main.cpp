#include <QApplication>
#include "exportpage.h"
#include "databasemanager.h"
#include <QStandardPaths>
#include <QDir>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 连数据库
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    DatabaseManager::instance().open(dataDir + "/lab_asset.db");

    ExportPage w;
    w.show();

    return a.exec();
}