#pragma once

#include <QDialog>

#include "equipment.h"
#include "equipmentrepository.h"
#include "equipment_global.h"

class QDateEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

class EQUIPMENT_EXPORT EquipmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EquipmentDialog(QWidget* parent = nullptr);
    explicit EquipmentDialog(const Equipment& equipment, QWidget* parent = nullptr);

    Equipment equipment() const;

protected:
    void accept() override;

private:
    void setupUi();
    void setEquipment(const Equipment& equipment);

    EquipmentRepository m_repository;
    int m_equipmentId = -1;
    LabAsset::EquipmentStatus m_status =
        LabAsset::EquipmentStatus::Available;

    QLineEdit* m_assetNoEdit = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QLineEdit* m_brandEdit = nullptr;
    QDateEdit* m_purchaseDateEdit = nullptr;
    QDoubleSpinBox* m_priceSpin = nullptr;
    QLineEdit* m_locationEdit = nullptr;
    QLabel* m_statusValueLabel = nullptr;
    QLineEdit* m_keeperEdit = nullptr;
    QPlainTextEdit* m_remarkEdit = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};
