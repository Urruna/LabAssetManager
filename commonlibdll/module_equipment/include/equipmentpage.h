#pragma once

#include <QList>
#include <QWidget>

#include "equipment.h"
#include "equipment_global.h"

class QSortFilterProxyModel;
class QStandardItemModel;

namespace Ui {
class EquipmentPage;
}

class EQUIPMENT_EXPORT EquipmentPage : public QWidget
{
    Q_OBJECT

public:
    explicit EquipmentPage(QWidget* parent = nullptr);
    ~EquipmentPage();

public slots:
    void refreshData();

signals:
    void dataChanged();

private slots:
    void addEquipment();
    void editEquipment();
    void deleteEquipment();
    void importEquipment();
    void downloadImportTemplate();
    void on_btn_export_equipment_clicked();

private:
    void setupTable();
    void populateTable();
    int selectedEquipmentId() const;
    const Equipment* findEquipment(int id) const;

    Ui::EquipmentPage* ui;
    QStandardItemModel* m_model = nullptr;
    QSortFilterProxyModel* m_proxy = nullptr;
    QList<Equipment> m_equipment;
};
