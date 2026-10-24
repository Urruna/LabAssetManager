#pragma once

#include <QList>
#include <QWidget>
#include "maintenancepository.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;

class MaintenancePage : public QWidget
{
    Q_OBJECT

public:
    explicit MaintenancePage(QWidget* parent = nullptr);

public slots:
    void refresh();

signals:
    void dataChanged();

private slots:
    void addRecord();
    void editRecord();

private:
    void setupUi();
    void refreshEquipmentFilter();
    void populateTable();

    MaintenancePository m_repository;
    QList<MaintenanceLedgerItem> m_records;

    QComboBox* m_equipmentFilter = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_editButton = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QTableWidget* m_table = nullptr;
    QLabel* m_statusLabel = nullptr;
};
