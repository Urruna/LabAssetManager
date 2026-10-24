#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "equipment.h"

class EquipmentRepository
{
public:
    explicit EquipmentRepository(
        QSqlDatabase database = QSqlDatabase::database("lab_asset_main"));

    QList<Equipment> find(
        const QString& keyword = QString(),
        const QString& status = QString()) const;
    bool assetNoExists(const QString& assetNo, int excludedId = -1) const;

    bool add(Equipment& equipment);
    bool update(Equipment& equipment);
    bool remove(int id);

    QString lastError() const;

private:
    bool validate(Equipment& equipment);
    QString equipmentStatus(int id, bool* ok) const;

    QSqlDatabase m_db;
    mutable QString m_lastError;
};
