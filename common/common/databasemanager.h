#pragma once
#include <QSqlDatabase>
#include <QString>
#include "common_global.h"

class COMMON_EXPORT DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool open(const QString& dbPath);
    void close();
    QSqlDatabase db() const { return m_db; }

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();

    QSqlDatabase m_db;
    static const QString kConnectionName;
};
