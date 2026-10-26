#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QString>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDate>
#include "enums.h"
#include "maintenancepository.h"

namespace {

bool taskTargetStatus(
    const QString& type,
    LabAsset::MaintenanceStatus status,
    LabAsset::EquipmentStatus& target)
{
    if (type == "repair" && status == LabAsset::MaintenanceStatus::Pending) {
        target = LabAsset::EquipmentStatus::PendingRepair;
        return true;
    }
    if (type == "repair" &&
        status == LabAsset::MaintenanceStatus::InProgress) {
        target = LabAsset::EquipmentStatus::Repairing;
        return true;
    }
    if (type == "maintain" &&
        status == LabAsset::MaintenanceStatus::InProgress) {
        target = LabAsset::EquipmentStatus::Maintaining;
        return true;
    }
    return false;
}

LabAsset::EquipmentStatus activeEquipmentStatus(const QString& type)
{
    return type == "maintain"
        ? LabAsset::EquipmentStatus::Maintaining
        : LabAsset::EquipmentStatus::Repairing;
}

} // namespace

MaintenancePository::MaintenancePository(QSqlDatabase database)
	: m_db(database)
{

}

QList<MaintenanceRecord> MaintenancePository::findAll(int equipmentId) const
{
    m_lastError.clear();
    QList<MaintenanceRecord> records; // 准备一个空列表装结果

    // 1. 拼接 SQL 语句
    QString sql = "SELECT id, equipment_id, maintenance_date, type, status, description, cost, operator "
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
        const auto status =
            LabAsset::maintenanceStatusFromDb(query.value("status").toString());
        if (!status) {
            m_lastError = "维保记录状态无效";
            return {};
        }
        record.status = *status;
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
        "SELECT m.id, m.equipment_id, m.maintenance_date, m.type, m.status, "
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
        const auto status =
            LabAsset::maintenanceStatusFromDb(query.value("status").toString());
        if (!status) {
            m_lastError = "维保台账中存在无效任务状态";
            return {};
        }
        item.record.status = *status;
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
        sql += "WHERE status = :available_status ";
    }
    sql += "ORDER BY asset_no ASC";

    QSqlQuery query(m_db);
    if (!query.prepare(sql)) {
        m_lastError = "准备查询设备列表失败: " + query.lastError().text();
        qDebug() << "SQL Prepare Error:" << m_lastError;
        return options;
    }

    if (availableOnly) {
        query.bindValue(
            ":available_status",
            LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));
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

bool MaintenancePository::add(MaintenanceRecord& record)
{
    m_lastError.clear();

    if (record.equipmentId <= 0) {
        m_lastError = "设备 ID 无效，无法登记维保";
        return false;
    }
    if (record.maintenanceDate.trimmed().isEmpty()) {
        m_lastError = "维保日期不能为空";
        return false;
    }

    LabAsset::EquipmentStatus targetStatus;
    if (!taskTargetStatus(record.type, record.status, targetStatus)) {
        m_lastError = "维保类型、任务状态或设备目标状态不匹配";
        return false;
    }

    if (!m_db.transaction()) {
        m_lastError = "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    QSqlQuery updateEquipment(m_db);
    updateEquipment.prepare(
        "UPDATE equipment "
        "SET status = :target_status "
        "WHERE id = :equipment_id "
        "AND status = :available_status "
        "AND NOT EXISTS ("
        "SELECT 1 FROM maintenance_record m "
        "WHERE m.equipment_id = equipment.id "
        "AND m.status IN ('pending', 'in_progress')"
        ")");
    updateEquipment.bindValue(
        ":target_status",
        LabAsset::statusToDb(targetStatus));
    updateEquipment.bindValue(":equipment_id", record.equipmentId);
    updateEquipment.bindValue(
        ":available_status",
        LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));

    if (!updateEquipment.exec() ||
        updateEquipment.numRowsAffected() != 1) {
        m_lastError =
            "设备当前不可用，或已有未完成的维修保养任务";
        m_db.rollback();
        return false;
    }

    QSqlQuery insertRecord(m_db);
    insertRecord.prepare(
        "INSERT INTO maintenance_record "
        "(equipment_id, maintenance_date, type, status, "
        " description, cost, operator) "
        "VALUES "
        "(:equipment_id, :maintenance_date, :type, :status, "
        " :description, :cost, :operator)");
    insertRecord.bindValue(":equipment_id", record.equipmentId);
    insertRecord.bindValue(":maintenance_date", record.maintenanceDate);
    insertRecord.bindValue(":type", record.type);
    insertRecord.bindValue(
        ":status",
        LabAsset::maintenanceStatusToDb(record.status));
    insertRecord.bindValue(":description", record.description);
    insertRecord.bindValue(":cost", record.cost);
    insertRecord.bindValue(":operator", record.operatorName);

    if (!insertRecord.exec()) {
        m_lastError =
            "插入维保记录失败: " + insertRecord.lastError().text();
        m_db.rollback();
        return false;
    }

    const int insertedId = insertRecord.lastInsertId().toInt();
    if (!m_db.commit()) {
        m_lastError =
            "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    record.id = insertedId;
    return true;
}

bool MaintenancePository::start(int id)
{
    m_lastError.clear();
    if (id <= 0) {
        m_lastError = "维保记录 ID 无效，无法开始处理";
        return false;
    }
    if (!m_db.transaction()) {
        m_lastError = "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    QSqlQuery recordQuery(m_db);
    recordQuery.prepare(
        "SELECT equipment_id, type, status "
        "FROM maintenance_record WHERE id = :id");
    recordQuery.bindValue(":id", id);
    if (!recordQuery.exec() || !recordQuery.next()) {
        m_lastError = "未找到对应的维保记录";
        m_db.rollback();
        return false;
    }

    const int equipmentId = recordQuery.value("equipment_id").toInt();
    const QString type = recordQuery.value("type").toString();
    const auto status =
        LabAsset::maintenanceStatusFromDb(
            recordQuery.value("status").toString());
    if (!status || *status != LabAsset::MaintenanceStatus::Pending) {
        m_lastError = "只有待处理任务可以开始处理";
        m_db.rollback();
        return false;
    }
    if (type != "repair") {
        m_lastError = "保养任务创建后直接进入处理中，无法再次开始";
        m_db.rollback();
        return false;
    }

    QSqlQuery equipmentQuery(m_db);
    equipmentQuery.prepare(
        "UPDATE equipment SET status = :new_status "
        "WHERE id = :equipment_id AND status = :expected_status");
    equipmentQuery.bindValue(
        ":new_status",
        LabAsset::statusToDb(LabAsset::EquipmentStatus::Repairing));
    equipmentQuery.bindValue(":equipment_id", equipmentId);
    equipmentQuery.bindValue(
        ":expected_status",
        LabAsset::statusToDb(LabAsset::EquipmentStatus::PendingRepair));
    if (!equipmentQuery.exec() || equipmentQuery.numRowsAffected() != 1) {
        m_lastError =
            "设备当前不是待维修状态，无法开始维修";
        m_db.rollback();
        return false;
    }

    QSqlQuery updateTask(m_db);
    updateTask.prepare(
        "UPDATE maintenance_record SET status = :new_status "
        "WHERE id = :id AND status = :expected_status");
    updateTask.bindValue(
        ":new_status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::InProgress));
    updateTask.bindValue(":id", id);
    updateTask.bindValue(
        ":expected_status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::Pending));
    if (!updateTask.exec() || updateTask.numRowsAffected() != 1) {
        m_lastError = "任务状态已变化，请刷新后重试";
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

bool MaintenancePository::complete(int id)
{
    m_lastError.clear();
    if (id <= 0) {
        m_lastError = "维保记录 ID 无效，无法完成处理";
        return false;
    }
    if (!m_db.transaction()) {
        m_lastError = "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    QSqlQuery recordQuery(m_db);
    recordQuery.prepare(
        "SELECT equipment_id, type, status "
        "FROM maintenance_record WHERE id = :id");
    recordQuery.bindValue(":id", id);
    if (!recordQuery.exec() || !recordQuery.next()) {
        m_lastError = "未找到对应的维保记录";
        m_db.rollback();
        return false;
    }

    const int equipmentId = recordQuery.value("equipment_id").toInt();
    const QString type = recordQuery.value("type").toString();
    const auto status =
        LabAsset::maintenanceStatusFromDb(
            recordQuery.value("status").toString());
    if (!status || *status != LabAsset::MaintenanceStatus::InProgress) {
        m_lastError = "只有处理中的任务可以完成";
        m_db.rollback();
        return false;
    }

    QSqlQuery equipmentQuery(m_db);
    equipmentQuery.prepare(
        "UPDATE equipment SET status = :new_status "
        "WHERE id = :equipment_id AND status = :expected_status");
    equipmentQuery.bindValue(
        ":new_status",
        LabAsset::statusToDb(LabAsset::EquipmentStatus::Available));
    equipmentQuery.bindValue(":equipment_id", equipmentId);
    equipmentQuery.bindValue(
        ":expected_status",
        LabAsset::statusToDb(activeEquipmentStatus(type)));
    if (!equipmentQuery.exec() || equipmentQuery.numRowsAffected() != 1) {
        m_lastError =
            "设备当前状态与任务不一致，无法完成处理";
        m_db.rollback();
        return false;
    }

    QSqlQuery updateTask(m_db);
    updateTask.prepare(
        "UPDATE maintenance_record SET status = :new_status "
        "WHERE id = :id AND status = :expected_status");
    updateTask.bindValue(
        ":new_status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::Completed));
    updateTask.bindValue(":id", id);
    updateTask.bindValue(
        ":expected_status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::InProgress));
    if (!updateTask.exec() || updateTask.numRowsAffected() != 1) {
        m_lastError = "任务状态已变化，请刷新后重试";
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

bool MaintenancePository::createPendingRepairForDamagedReturn(int equipmentId)
{
    m_lastError.clear();
    if (equipmentId <= 0) {
        m_lastError = "设备 ID 无效，无法创建待维修任务";
        return false;
    }
    if (!m_db.transaction()) {
        m_lastError = "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    QSqlQuery equipmentQuery(m_db);
    equipmentQuery.prepare(
        "SELECT status FROM equipment WHERE id = :id");
    equipmentQuery.bindValue(":id", equipmentId);
    if (!equipmentQuery.exec() || !equipmentQuery.next() ||
        equipmentQuery.value("status").toString() !=
            LabAsset::statusToDb(LabAsset::EquipmentStatus::PendingRepair)) {
        m_lastError = "设备不是待维修状态，无法创建损坏归还任务";
        m_db.rollback();
        return false;
    }

    QSqlQuery activeQuery(m_db);
    activeQuery.prepare(
        "SELECT 1 FROM maintenance_record "
        "WHERE equipment_id = :equipment_id "
        "AND status IN ('pending', 'in_progress') LIMIT 1");
    activeQuery.bindValue(":equipment_id", equipmentId);
    if (!activeQuery.exec()) {
        m_lastError = "检查未完成维保任务失败";
        m_db.rollback();
        return false;
    }
    if (activeQuery.next()) {
        m_lastError = "设备已有未完成的维修保养任务";
        m_db.rollback();
        return false;
    }

    QSqlQuery insertRecord(m_db);
    insertRecord.prepare(
        "INSERT INTO maintenance_record "
        "(equipment_id, maintenance_date, type, status, "
        " description, cost, operator) "
        "VALUES "
        "(:equipment_id, :maintenance_date, 'repair', :status, "
        " :description, 0, :operator)");
    insertRecord.bindValue(":equipment_id", equipmentId);
    insertRecord.bindValue(
        ":maintenance_date",
        QDate::currentDate().toString("yyyy-MM-dd"));
    insertRecord.bindValue(
        ":status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::Pending));
    insertRecord.bindValue(":description", "设备归还时标记损坏");
    insertRecord.bindValue(":operator", "系统");
    if (!insertRecord.exec()) {
        m_lastError =
            "创建待维修任务失败: " + insertRecord.lastError().text();
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    return true;
}

bool MaintenancePository::update(const MaintenanceRecord& record)
{
    m_lastError.clear();

    if (record.id <= 0 || record.equipmentId <= 0) {
        m_lastError = "维保记录或设备 ID 无效，无法更新";
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

    if (!m_db.transaction()) {
        m_lastError = "无法开启数据库事务: " + m_db.lastError().text();
        return false;
    }

    QSqlQuery currentQuery(m_db);
    currentQuery.prepare(
        "SELECT equipment_id, type, status "
        "FROM maintenance_record WHERE id = :id");
    currentQuery.bindValue(":id", record.id);
    if (!currentQuery.exec() || !currentQuery.next()) {
        m_lastError = "未找到对应的维保记录";
        m_db.rollback();
        return false;
    }

    const int currentEquipmentId =
        currentQuery.value("equipment_id").toInt();
    const QString currentType = currentQuery.value("type").toString();
    const QString currentStatus = currentQuery.value("status").toString();
    if (currentStatus !=
            LabAsset::maintenanceStatusToDb(
                LabAsset::MaintenanceStatus::Completed) &&
        (record.equipmentId != currentEquipmentId ||
         record.type != currentType)) {
        m_lastError =
            "未完成任务只能修改日期、说明、费用和经办人";
        m_db.rollback();
        return false;
    }

    QSqlQuery updateRecord(m_db);
    updateRecord.prepare(
        "UPDATE maintenance_record "
        "SET equipment_id = :equipment_id, "
        "    maintenance_date = :maintenance_date, "
        "    type = :type, "
        "    description = :description, "
        "    cost = :cost, "
        "    operator = :operator "
        "WHERE id = :id AND status = :current_status");
    updateRecord.bindValue(":equipment_id", record.equipmentId);
    updateRecord.bindValue(":maintenance_date", record.maintenanceDate);
    updateRecord.bindValue(":type", record.type);
    updateRecord.bindValue(":description", record.description);
    updateRecord.bindValue(":cost", record.cost);
    updateRecord.bindValue(":operator", record.operatorName);
    updateRecord.bindValue(":id", record.id);
    updateRecord.bindValue(":current_status", currentStatus);

    if (!updateRecord.exec() || updateRecord.numRowsAffected() != 1) {
        m_lastError = "更新维保记录失败或记录状态已变化";
        m_db.rollback();
        return false;
    }

    if (!m_db.commit()) {
        m_lastError = "提交事务失败: " + m_db.lastError().text();
        m_db.rollback();
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

    // 未完成任务不能直接删除，避免设备状态与台账失去对应关系。
    QSqlQuery query(m_db);
    query.prepare(
        "DELETE FROM maintenance_record "
        "WHERE id = :id AND status = :completed_status");

    query.bindValue(":id", id);
    query.bindValue(
        ":completed_status",
        LabAsset::maintenanceStatusToDb(
            LabAsset::MaintenanceStatus::Completed));

    if (!query.exec()) {
        m_lastError =
            "删除维保记录失败: " + query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() != 1) {
        m_lastError = "只能删除已完成记录，或记录已不存在";
        return false;
    }

    return true;
}

QString MaintenancePository::lastError() const
{
    return m_lastError;
}
