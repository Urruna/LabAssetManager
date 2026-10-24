#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include "maintenancepository.h"

MaintenancePository::MaintenancePository(QSqlDatabase database)
	: m_db(database)
{

}

QList<MaintenanceRecord> MaintenancePository::findAll(int equipmentId) const
{
    m_lastError.clear();
    QList<MaintenanceRecord> records; // 准备一个空列表装结果

    // 1. 拼接 SQL 语句
    QString sql = "SELECT id, equipment_id, maintenance_date, type, description, cost, operator "
        "FROM maintenance_record ";

    // 指定了 equipmentId（不为 -1）时，增加 WHERE 条件
    if (equipmentId != -1) {
        sql += "WHERE equipment_id = :equipment_id ";
    }

    // 按照最近日期优先排序
    sql += "ORDER BY maintenance_date DESC, id DESC";

    // 2. 准备执行查询
    QSqlQuery query(m_db);
    if (!query.prepare(sql)) {
        m_lastError = "准备查询维保记录失败: " + query.lastError().text();
        qDebug() << "SQL Prepare Error:" << m_lastError;
        return records;
    }

    // 3. 绑定设备 ID 参数
    if (equipmentId != -1) {
        query.bindValue(":equipment_id", equipmentId);
    }

    // 4. 执行查询
    if (!query.exec()) {
        m_lastError = "执行维保记录查询失败: " + query.lastError().text();
        qDebug() << "SQL Exec Error:" << m_lastError;
        return records;
    }

    // 5. 读取查询结果并转换为记录结构
    while (query.next()) {
        MaintenanceRecord record;

        record.id = query.value("id").toInt();
        record.equipmentId = query.value("equipment_id").toInt();
        record.maintenanceDate = query.value("maintenance_date").toString();
        record.type = query.value("type").toString();
        record.description = query.value("description").toString();
        record.cost = query.value("cost").toDouble();
        record.operatorName = query.value("operator").toString();

        records.append(record);
    }

    // 6. 返回记录列表
    return records;
}

QList<MaintenanceLedgerItem> MaintenancePository::findLedger(int equipmentId) const
{
    m_lastError.clear();
    QList<MaintenanceLedgerItem> records;

    // 台账页面需要显示设备编号和名称，因此联表查询 equipment。
    QString sql =
        "SELECT m.id, m.equipment_id, m.maintenance_date, m.type, "
        "       m.description, m.cost, m.operator, "
        "       e.asset_no, e.name "
        "FROM maintenance_record m "
        "INNER JOIN equipment e ON e.id = m.equipment_id ";

    if (equipmentId != -1) {
        sql += "WHERE m.equipment_id = :equipment_id ";
    }

    sql += "ORDER BY m.maintenance_date DESC, m.id DESC";

    QSqlQuery query(m_db);
    if (!query.prepare(sql)) {
        m_lastError = "准备查询维保台账失败: " + query.lastError().text();
        qDebug() << "SQL Prepare Error:" << m_lastError;
        return records;
    }

    if (equipmentId != -1) {
        query.bindValue(":equipment_id", equipmentId);
    }

    if (!query.exec()) {
        m_lastError = "执行维保台账查询失败: " + query.lastError().text();
        qDebug() << "SQL Exec Error:" << m_lastError;
        return records;
    }

    while (query.next()) {
        MaintenanceLedgerItem item;

        item.record.id = query.value("id").toInt();
        item.record.equipmentId = query.value("equipment_id").toInt();
        item.record.maintenanceDate =
            query.value("maintenance_date").toString();
        item.record.type = query.value("type").toString();
        item.record.description = query.value("description").toString();
        item.record.cost = query.value("cost").toDouble();
        item.record.operatorName = query.value("operator").toString();
        item.assetNo = query.value("asset_no").toString();
        item.equipmentName = query.value("name").toString();

        records.append(item);
    }

    return records;
}

QList<MaintenanceEquipmentOption>
MaintenancePository::findEquipmentOptions(bool availableOnly) const
{
    m_lastError.clear();
    QList<MaintenanceEquipmentOption> options;

    QString sql = "SELECT id, asset_no, name FROM equipment ";
    if (availableOnly) {
        sql += "WHERE status = 'available' ";
    }
    sql += "ORDER BY asset_no ASC";

    QSqlQuery query(m_db);
    if (!query.prepare(sql)) {
        m_lastError = "准备查询设备列表失败: " + query.lastError().text();
        qDebug() << "SQL Prepare Error:" << m_lastError;
        return options;
    }

    if (!query.exec()) {
        m_lastError = "执行设备列表查询失败: " + query.lastError().text();
        qDebug() << "SQL Exec Error:" << m_lastError;
        return options;
    }

    while (query.next()) {
        MaintenanceEquipmentOption option;
        option.id = query.value("id").toInt();
        option.assetNo = query.value("asset_no").toString();
        option.name = query.value("name").toString();
        options.append(option);
    }

    return options;
}

QMap<QString, int> MaintenancePository::countEquipmentByStatus() const
{
    m_lastError.clear();
    QMap<QString, int> counts;

    QSqlQuery query(m_db);
    if (!query.prepare(
            "SELECT status, COUNT(*) AS total "
            "FROM equipment "
            "GROUP BY status")) {
        m_lastError = "准备统计设备状态失败: " + query.lastError().text();
        qDebug() << "SQL Prepare Error:" << m_lastError;
        return counts;
    }

    if (!query.exec()) {
        m_lastError = "执行设备状态统计失败: " + query.lastError().text();
        qDebug() << "SQL Exec Error:" << m_lastError;
        return counts;
    }

    while (query.next()) {
        counts.insert(
            query.value("status").toString(),
            query.value("total").toInt());
    }

    return counts;
}

bool MaintenancePository::add(MaintenanceRecord& record, const QString& equipmentStatus)
{
    m_lastError.clear();

    // 1. 基础字段校验
    if (record.equipmentId <= 0) {
        m_lastError = "设备 ID 无效，无法登记维保";
        return false;
    }

    if (record.maintenanceDate.trimmed().isEmpty()) {
        m_lastError = "维保日期不能为空";
        return false;
    }

    // 2. 检查记录类型与目标设备状态是否匹配
    bool validTargetStatus = false;

    if (record.type == "repair") {
        validTargetStatus =
            equipmentStatus == "pending_repair" ||
            equipmentStatus == "repairing";
    }
    else if (record.type == "maintain") {
        validTargetStatus = equipmentStatus == "under_maintenance";
    }

    if (!validTargetStatus) {
        m_lastError =
            "维保类型与目标设备状态不匹配，无法登记维保";
        return false;
    }

    // 3. 开启事务
    if (!m_db.transaction()) {
        m_lastError =
            "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    // 4. 条件更新设备状态
    // 只有设备当前状态为 available 时，才允许进入维保流程。
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare(
        "UPDATE equipment "
        "SET status = :target_status "
        "WHERE id = :equipment_id AND status = 'available'");

    updateQuery.bindValue(":target_status", equipmentStatus);
    updateQuery.bindValue(":equipment_id", record.equipmentId);

    if (!updateQuery.exec()) {
        m_lastError =
            "更新设备状态失败: " + updateQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    if (updateQuery.numRowsAffected() != 1) {
        m_lastError =
            "设备当前不是可用状态，可能已借出、维修中或已报废";
        m_db.rollback();
        return false;
    }

    // 5. 插入维保记录
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare(
        "INSERT INTO maintenance_record "
        "(equipment_id, maintenance_date, type, description, cost, operator) "
        "VALUES "
        "(:equipment_id, :maintenance_date, :type, :description, :cost, :operator)");

    insertQuery.bindValue(":equipment_id", record.equipmentId);
    insertQuery.bindValue(":maintenance_date", record.maintenanceDate);
    insertQuery.bindValue(":type", record.type);
    insertQuery.bindValue(":description", record.description);
    insertQuery.bindValue(":cost", record.cost);
    insertQuery.bindValue(":operator", record.operatorName);

    if (!insertQuery.exec()) {
        m_lastError =
            "插入维保记录失败: " + insertQuery.lastError().text();
        m_db.rollback();
        return false;
    }

    int insertedId = insertQuery.lastInsertId().toInt();

    // 6. 提交事务
    if (!m_db.commit()) {
        m_lastError =
            "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    // 只有事务提交成功后才回填记录 ID
    record.id = insertedId;
    return true;
}

bool MaintenancePository::update(const MaintenanceRecord& record)
{
    m_lastError.clear();

    // 1. 基础字段校验
    if (record.id <= 0) {
        m_lastError = "维保记录 ID 无效，无法更新";
        return false;
    }

    if (record.equipmentId <= 0) {
        m_lastError = "设备 ID 无效，无法更新维保记录";
        return false;
    }

    if (record.maintenanceDate.trimmed().isEmpty()) {
        m_lastError = "维保日期不能为空";
        return false;
    }

    if (record.type != "repair" && record.type != "maintain") {
        m_lastError = "维保类型无效，必须为 repair 或 maintain";
        return false;
    }

    // 2. 更新台账记录
    // 台账修改只纠正历史记录内容，不联动修改 equipment.status。
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE maintenance_record "
        "SET equipment_id = :equipment_id, "
        "    maintenance_date = :maintenance_date, "
        "    type = :type, "
        "    description = :description, "
        "    cost = :cost, "
        "    operator = :operator "
        "WHERE id = :id");

    query.bindValue(":equipment_id", record.equipmentId);
    query.bindValue(":maintenance_date", record.maintenanceDate);
    query.bindValue(":type", record.type);
    query.bindValue(":description", record.description);
    query.bindValue(":cost", record.cost);
    query.bindValue(":operator", record.operatorName);
    query.bindValue(":id", record.id);

    if (!query.exec()) {
        m_lastError =
            "更新维保记录失败: " + query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        m_lastError = "未找到对应的维保记录，可能已被删除";
        return false;
    }

    return true;
}

bool MaintenancePository::remove(int id)
{
    m_lastError.clear();

    if (id <= 0) {
        m_lastError = "维保记录 ID 无效，无法删除";
        return false;
    }

    // 删除只移除台账记录，不联动修改 equipment.status。
    QSqlQuery query(m_db);
    query.prepare(
        "DELETE FROM maintenance_record "
        "WHERE id = :id");

    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError =
            "删除维保记录失败: " + query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        m_lastError = "未找到对应的维保记录，可能已被删除";
        return false;
    }

    return true;
}

QString MaintenancePository::lastError() const
{
    return m_lastError;
}
