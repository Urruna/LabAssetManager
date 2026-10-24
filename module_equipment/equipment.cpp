#include "equipment.h"

const QStringList& equipmentStatusValues()
{
    static const QStringList values = {
        "available",
        "borrowed",
        "pending_repair",
        "repairing",
        "under_maintenance",
        "scrapped"
    };
    return values;
}

QString equipmentStatusText(const QString& status)
{
    if (status == "available") return "在用";
    if (status == "borrowed") return "借出";
    if (status == "pending_repair") return "待维修";
    if (status == "repairing") return "维修中";
    if (status == "under_maintenance") return "保养中";
    if (status == "scrapped") return "已报废";
    return status;
}
