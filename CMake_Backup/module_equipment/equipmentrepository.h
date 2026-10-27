#pragma once

#include <QList>
#include <QSqlDatabase>
#include <QString>

#include <optional>

#include "equipment.h"

class EquipmentRepository
{
public:
    explicit EquipmentRepository(
        QSqlDatabase database = QSqlDatabase::database("lab_asset_main"));

    QList<Equipment> find(
        const QString& keyword = QString(),
        std::optional<LabAsset::EquipmentStatus> status = std::nullopt) const;
    bool assetNoExists(const QString& assetNo, int excludedId = -1) const;

    bool add(Equipment& equipment);
    bool addBatch(QList<Equipment>& equipments, QString& error);
    bool update(Equipment& equipment);
    bool remove(int id);

    QString lastError() const;

private:
    bool validate(Equipment& equipment);
    bool equipmentStatus(
        int id,
        LabAsset::EquipmentStatus& status) const;

    QSqlDatabase m_db;
    mutable QString m_lastError;
};
