#pragma once

#include <QList>
#include <QWidget>
#include "maintenancepository.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MaintenancePage; }
QT_END_NAMESPACE

class MaintenancePage : public QWidget
{
    Q_OBJECT

public:
    explicit MaintenancePage(QWidget* parent = nullptr);
    ~MaintenancePage() override;

public slots:
    void refreshData();
    bool createPendingRepairForDamagedReturn(int equipmentId);

signals:
    void dataChanged();

private slots:
    void addRecord();
    void editRecord();
    void startRecord();
    void completeRecord();
    void exportData();

private:
    void setupUi();
    void refreshEquipmentFilter();
    void populateTable();
    void updateActionButtons();

    MaintenancePository m_repository;
    QList<MaintenanceLedgerItem> m_records;
    Ui::MaintenancePage* ui;
};
