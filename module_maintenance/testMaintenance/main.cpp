#include <QApplication>
#include <QTemporaryDir>

#include "databasemanager.h"
#include "maintenancepage.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid() ||
        !DatabaseManager::instance().open(directory.path() + "/test.db")) {
        return 1;
    }

    MaintenancePage page;
    page.show();
    const int result = app.exec();
    DatabaseManager::instance().close();
    return result;
}
