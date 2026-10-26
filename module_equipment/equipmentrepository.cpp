#include "equipmentrepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <algorithm>
#include <cmath>

namespace {

QString escapeLikePattern(QString text)
{
    text.replace("\\", "\\\\");
    text.replace("%", "\\%");
    text.replace("_", "\\_");
    return text;
}

bool equipmentFromQuery(
    const QSqlQuery& query,
    Equipment& equipment,
    QString& error)
{
    equipment.id = query.value(0).toInt();
    equipment.assetNo = query.value(1).toString();
    equipment.name = query.value(2).toString();
    equipment.model = query.value(3).toString();
    equipment.brand = query.value(4).toString();
    equipment.purchaseDate = query.value(5).toString();
    equipment.price = query.value(6).toDouble();
    equipment.location = query.value(7).toString();
    const QString rawStatus = query.value(8).toString();
    const auto parsedStatus = LabAsset::statusFromDb(rawStatus);
    if (!parsedStatus) {
        error = QString("设备“%1”的状态无效：%2")
                    .arg(equipment.assetNo, rawStatus);
        return false;
    }
    equipment.status = *parsedStatus;
    equipment.keeper = query.value(9).toString();
    equipment.remark = query.value(10).toString();
    return true;
}

} // namespace

EquipmentRepository::EquipmentRepository(QSqlDatabase database)
    : m_db(database)
{
}

QList<Equipment> EquipmentRepository::find(
    const QString& keyword,
    std::optional<LabAsset::EquipmentStatus> status) const
{
    m_lastError.clear();
    QList<Equipment> result;

    QString sql =
        "SELECT id, asset_no, name, model, brand, purchase_date, "
        "       price, location, status, keeper, remark "
        "FROM equipment ";
    QStringList conditions;

    const QString trimmedKeyword = keyword.trimmed();
    if (!trimmedKeyword.isEmpty()) {
        conditions.append(
            "(asset_no LIKE :keyword ESCAPE '\\' "
            "OR name LIKE :keyword ESCAPE '\\')");
    }

    if (status.has_value()) {
        conditions.append("status = :status");
    }

    if (!conditions.isEmpty()) {
        sql += "WHERE " + conditions.join(" AND ") + " ";
    }
    sql += "ORDER BY asset_no ASC, id ASC";

    QSqlQuery query(m_db);
    if (!query.prepare(sql)) {
        m_lastError = "准备设备查询失败：" + query.lastError().text();
        return result;
    }

    if (!trimmedKeyword.isEmpty()) {
        const QString pattern =
            "%" + escapeLikePattern(trimmedKeyword) + "%";
        query.bindValue(":keyword", pattern);
    }
    if (status.has_value()) {
        query.bindValue(
            ":status",
            LabAsset::statusToDb(*status));
    }

    if (!query.exec()) {
        m_lastError = "查询设备失败：" + query.lastError().text();
        return result;
    }

    while (query.next()) {
        Equipment equipment;
        QString error;
        if (!equipmentFromQuery(query, equipment, error)) {
            m_lastError = error;
            return {};
        }
        result.append(equipment);
    }
    return result;
}

bool EquipmentRepository::assetNoExists(
    const QString& assetNo,
    int excludedId) const
{
    m_lastError.clear();

    QSqlQuery query(m_db);
    if (!query.prepare(
            "SELECT 1 FROM equipment "
            "WHERE asset_no = :asset_no AND id <> :excluded_id "
            "LIMIT 1")) {
        m_lastError =
            "准备资产编号校验失败：" + query.lastError().text();
        return false;
    }

    query.bindValue(":asset_no", assetNo.trimmed());
    query.bindValue(":excluded_id", excludedId);

    if (!query.exec()) {
        m_lastError = "校验资产编号失败：" + query.lastError().text();
        return false;
    }
    return query.next();
}

bool EquipmentRepository::add(Equipment& equipment)
{
    m_lastError.clear();
    if (!validate(equipment)) {
        return false;
    }

    if (assetNoExists(equipment.assetNo)) {
        m_lastError = "资产编号已存在，请使用其他编号";
        return false;
    }
    if (!m_lastError.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.prepare(
            "INSERT INTO equipment "
            "(asset_no, name, model, brand, purchase_date, price, "
            " location, status, keeper, remark) "
            "VALUES "
            "(:asset_no, :name, :model, :brand, :purchase_date, :price, "
            " :location, :status, :keeper, :remark)")) {
        m_lastError = "准备新增设备失败：" + query.lastError().text();
        return false;
    }

    query.bindValue(":asset_no", equipment.assetNo);
    query.bindValue(":name", equipment.name);
    query.bindValue(":model", equipment.model);
    query.bindValue(":brand", equipment.brand);
    query.bindValue(":purchase_date", equipment.purchaseDate);
    query.bindValue(":price", equipment.price);
    query.bindValue(":location", equipment.location);
    query.bindValue(
        ":status",
        LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));
    query.bindValue(":keeper", equipment.keeper);
    query.bindValue(":remark", equipment.remark);

    if (!query.exec()) {
        const QString detail = query.lastError().text();
        if (detail.contains("UNIQUE", Qt::CaseInsensitive)) {
            m_lastError = "资产编号已存在，请使用其他编号";
        }
        else {
            m_lastError = "新增设备失败：" + detail;
        }
        return false;
    }

    equipment.id = query.lastInsertId().toInt();
    equipment.status = LabAsset::EquipmentStatus::Available;
    return true;
}

bool EquipmentRepository::update(Equipment& equipment)
{
    m_lastError.clear();
    if (equipment.id <= 0) {
        m_lastError = "设备 ID 无效，无法更新";
        return false;
    }
    if (!validate(equipment)) {
        return false;
    }

    if (assetNoExists(equipment.assetNo, equipment.id)) {
        m_lastError = "资产编号已存在，请使用其他编号";
        return false;
    }
    if (!m_lastError.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.prepare(
            "UPDATE equipment SET "
            "asset_no = :asset_no, name = :name, model = :model, "
            "brand = :brand, purchase_date = :purchase_date, "
            "price = :price, location = :location, "
            "keeper = :keeper, remark = :remark "
            "WHERE id = :id")) {
        m_lastError = "准备更新设备失败：" + query.lastError().text();
        return false;
    }

    query.bindValue(":asset_no", equipment.assetNo);
    query.bindValue(":name", equipment.name);
    query.bindValue(":model", equipment.model);
    query.bindValue(":brand", equipment.brand);
    query.bindValue(":purchase_date", equipment.purchaseDate);
    query.bindValue(":price", equipment.price);
    query.bindValue(":location", equipment.location);
    query.bindValue(":keeper", equipment.keeper);
    query.bindValue(":remark", equipment.remark);
    query.bindValue(":id", equipment.id);

    if (!query.exec()) {
        const QString detail = query.lastError().text();
        if (detail.contains("UNIQUE", Qt::CaseInsensitive)) {
            m_lastError = "资产编号已存在，请使用其他编号";
        }
        else {
            m_lastError = "更新设备失败：" + detail;
        }
        return false;
    }

    if (query.numRowsAffected() != 1) {
        m_lastError = "未找到对应设备，记录可能已被删除";
        return false;
    }
    return true;
}

bool EquipmentRepository::remove(int id)
{
    m_lastError.clear();
    if (id <= 0) {
        m_lastError = "设备 ID 无效，无法删除";
        return false;
    }

    LabAsset::EquipmentStatus currentStatus;
    if (!equipmentStatus(id, currentStatus)) {
        if (m_lastError.isEmpty()) {
            m_lastError = "未找到对应设备，记录可能已被删除";
        }
        return false;
    }

    // 非在用设备可能正在借出、维修或保养，先按业务状态拒绝删除。
    if (currentStatus != LabAsset::EquipmentStatus::Available) {
        m_lastError = QString(
            "该设备当前为“%1”，存在关联流程，无法删除")
            .arg(LabAsset::statusToDisplay(currentStatus));
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.prepare("DELETE FROM equipment WHERE id = :id")) {
        m_lastError = "准备删除设备失败：" + query.lastError().text();
        return false;
    }
    query.bindValue(":id", id);

    if (!query.exec()) {
        const QString detail = query.lastError().text();
        if (detail.contains("FOREIGN KEY", Qt::CaseInsensitive)) {
            m_lastError =
                "该设备存在关联借还记录，无法删除";
        }
        else {
            m_lastError = "删除设备失败：" + detail;
        }
        return false;
    }

    if (query.numRowsAffected() != 1) {
        m_lastError = "未找到对应设备，记录可能已被删除";
        return false;
    }
    return true;
}

QString EquipmentRepository::lastError() const
{
    return m_lastError;
}

bool EquipmentRepository::validate(Equipment& equipment)
{
    equipment.assetNo = equipment.assetNo.trimmed();
    equipment.name = equipment.name.trimmed();
    equipment.model = equipment.model.trimmed();
    equipment.brand = equipment.brand.trimmed();
    equipment.purchaseDate = equipment.purchaseDate.trimmed();
    equipment.location = equipment.location.trimmed();
    equipment.keeper = equipment.keeper.trimmed();
    equipment.remark = equipment.remark.trimmed();

    if (equipment.assetNo.isEmpty()) {
        m_lastError = "资产编号不能为空";
        return false;
    }
    if (equipment.name.isEmpty()) {
        m_lastError = "设备名称不能为空";
        return false;
    }
    if (!std::isfinite(equipment.price) || equipment.price < 0.0) {
        m_lastError = "价格必须是合法的非负数字";
        return false;
    }
    const auto& statuses = LabAsset::allEquipmentStatuses();
    if (std::find(
            statuses.cbegin(),
            statuses.cend(),
            equipment.status) == statuses.cend()) {
        m_lastError = "设备状态无效";
        return false;
    }
    return true;
}

bool EquipmentRepository::equipmentStatus(
    int id,
    LabAsset::EquipmentStatus& status) const
{
    QSqlQuery query(m_db);
    if (!query.prepare("SELECT status FROM equipment WHERE id = :id")) {
        m_lastError = "准备读取设备状态失败：" + query.lastError().text();
        return false;
    }
    query.bindValue(":id", id);
    if (!query.exec()) {
        m_lastError = "读取设备状态失败：" + query.lastError().text();
        return false;
    }
    if (!query.next()) {
        return false;
    }

    const QString rawStatus = query.value(0).toString();
    const auto parsedStatus = LabAsset::statusFromDb(rawStatus);
    if (!parsedStatus) {
        m_lastError = "设备状态无效：" + rawStatus;
        return false;
    }

    status = *parsedStatus;
    return true;
}
