#pragma once

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QString>
#include "enums.h"
#include "maintenancerecord.h"

struct MaintenanceEquipmentOption
{
    int id = -1;
    QString assetNo;
    QString name;
};

struct MaintenanceLedgerItem
{
    MaintenanceRecord record;
    QString assetNo;
    QString equipmentName;
};

class MaintenancePository
{
public:
    explicit MaintenancePository(
        QSqlDatabase database = QSqlDatabase::database("lab_asset_main"));

    QList<MaintenanceRecord> findAll(int equipmentId = -1) const;
    QList<MaintenanceLedgerItem> findLedger(int equipmentId = -1) const;
    QList<MaintenanceEquipmentOption> findEquipmentOptions(
        bool availableOnly = false) const;
    QMap<QString, int> countEquipmentByStatus() const;

    bool add(
        MaintenanceRecord& record,
        LabAsset::EquipmentStatus equipmentStatus);
    bool update(const MaintenanceRecord& record);
    bool remove(int id);

    QString lastError() const;

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};
