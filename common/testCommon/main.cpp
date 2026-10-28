#include <QCoreApplication>
#include <QTemporaryDir>

#include "databasemanager.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid()) {
        return 1;
    }
    const bool opened =
        DatabaseManager::instance().open(directory.path() + "/test.db");
    DatabaseManager::instance().close();
    return opened ? 0 : 1;
}
