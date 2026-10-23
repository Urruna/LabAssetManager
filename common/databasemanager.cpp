#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

const QString DatabaseManager::kConnectionName = "lab_asset_main";

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString& dbPath)
{
    if (m_db.isOpen())
        return true;

    m_db = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "打开数据库失败:" << m_db.lastError().text();
        return false;
    }

    // SQLite 每个连接都要手动打开外键约束
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON");

    return createTables();
}

void DatabaseManager::close()
{
    if (m_db.isOpen())
        m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(kConnectionName);
}

bool DatabaseManager::createTables()
{
    const QStringList stmts = {
        R"(CREATE TABLE IF NOT EXISTS equipment (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            asset_no      TEXT NOT NULL UNIQUE,
            name          TEXT NOT NULL,
            model         TEXT,
            brand         TEXT,
            purchase_date TEXT,
            price         REAL DEFAULT 0,
            location      TEXT,
            status        TEXT NOT NULL DEFAULT 'available',
            keeper        TEXT,
            remark        TEXT
        ))",
        R"(CREATE TABLE IF NOT EXISTS borrow_record (
            id                   INTEGER PRIMARY KEY AUTOINCREMENT,
            equipment_id         INTEGER NOT NULL,
            borrower             TEXT NOT NULL,
            borrow_time          TEXT NOT NULL,
            expected_return_time TEXT,
            actual_return_time   TEXT,
            status               TEXT NOT NULL DEFAULT 'borrowed',
            remark               TEXT,
            FOREIGN KEY (equipment_id) REFERENCES equipment(id) ON DELETE RESTRICT
        ))",
        R"(CREATE TABLE IF NOT EXISTS maintenance_record (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            equipment_id     INTEGER NOT NULL,
            maintenance_date TEXT NOT NULL,
            type             TEXT NOT NULL,
            description      TEXT,
            cost             REAL DEFAULT 0,
            operator         TEXT,
            FOREIGN KEY (equipment_id) REFERENCES equipment(id) ON DELETE CASCADE
        ))",
        "CREATE INDEX IF NOT EXISTS idx_equipment_asset_no ON equipment(asset_no)",
        "CREATE INDEX IF NOT EXISTS idx_borrow_equipment ON borrow_record(equipment_id)",
        "CREATE INDEX IF NOT EXISTS idx_maintenance_equipment ON maintenance_record(equipment_id)"
    };

    QSqlQuery q(m_db);
    for (const QString& sql : stmts) {
        if (!q.exec(sql)) {
            qCritical() << "建表失败:" << sql << q.lastError().text();
            return false;
        }
    }
    return true;
}