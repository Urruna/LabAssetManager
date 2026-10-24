#pragma once

#include <QWidget>
#include "maintenancepository.h"

class QLabel;
class QPushButton;
class QTableWidget;

class StatisticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPage(QWidget* parent = nullptr);

public slots:
    void refreshStatistics();

private:
    void setupUi();

    MaintenancePository m_repository;
    QLabel* m_totalLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QTableWidget* m_table = nullptr;
};
