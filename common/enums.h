#pragma once
#include <QString>
#include <array>
#include <optional>

namespace LabAsset {

    enum class EquipmentStatus {
        Available,   // 在用
        Borrowed,    // 借出
        PendingRepair, // 待维修
        Repairing,   // 维修中
        Maintaining, // 保养中
        Scrapped     // 已报废
    };

    inline const std::array<EquipmentStatus, 6>& allEquipmentStatuses() {
        static const std::array<EquipmentStatus, 6> statuses = {
            EquipmentStatus::Available,
            EquipmentStatus::Borrowed,
            EquipmentStatus::PendingRepair,
            EquipmentStatus::Repairing,
            EquipmentStatus::Maintaining,
            EquipmentStatus::Scrapped
        };
        return statuses;
    }

    // 存数据库时的英文字符串 (核心契约)
    inline QString statusToDb(EquipmentStatus s) {
        switch (s) {
        case EquipmentStatus::Available:     return "available";
        case EquipmentStatus::Borrowed:      return "borrowed";
        case EquipmentStatus::PendingRepair: return "pending_repair";
        case EquipmentStatus::Repairing:     return "repairing";
        case EquipmentStatus::Maintaining:   return "maintaining";
        case EquipmentStatus::Scrapped:      return "scrapped";
        }
        return {};
    }

    // 从数据库读出来的转换
    inline std::optional<EquipmentStatus> statusFromDb(const QString& s) {
        if (s == "available")      return EquipmentStatus::Available;
        if (s == "borrowed")       return EquipmentStatus::Borrowed;
        if (s == "pending_repair") return EquipmentStatus::PendingRepair;
        if (s == "repairing")      return EquipmentStatus::Repairing;
        if (s == "maintaining")    return EquipmentStatus::Maintaining;
        if (s == "scrapped")       return EquipmentStatus::Scrapped;
        return std::nullopt;
    }

    // 界面显示的中文
    inline QString statusToDisplay(EquipmentStatus s) {
        switch (s) {
        case EquipmentStatus::Available:     return QStringLiteral("在用");
        case EquipmentStatus::Borrowed:      return QStringLiteral("借出");
        case EquipmentStatus::PendingRepair: return QStringLiteral("待维修");
        case EquipmentStatus::Repairing:     return QStringLiteral("维修中");
        case EquipmentStatus::Maintaining:   return QStringLiteral("保养中");
        case EquipmentStatus::Scrapped:      return QStringLiteral("已报废");
        }
        return {};
    }

}
