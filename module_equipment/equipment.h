#pragma once

#include <QString>
#include <QStringList>

struct Equipment
{
    int id = -1;
    QString assetNo;
    QString name;
    QString model;
    QString brand;
    QString purchaseDate;
    double price = 0.0;
    QString location;
    QString status = "available";
    QString keeper;
    QString remark;
};

// 状态值统一使用数据库中的英文常量，界面只展示中文文本。
const QStringList& equipmentStatusValues();
QString equipmentStatusText(const QString& status);
