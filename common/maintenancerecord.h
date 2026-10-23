#pragma once

#include <QString>

struct MaintenanceRecord
{
    int id = -1;
    int equipmentId = -1;
    QString maintenanceDate;
    QString type; // repair: Î¬ÐÞ, maintain: Î¬±£
    QString description;
    double cost = 0.0;
    QString operatorName; 
