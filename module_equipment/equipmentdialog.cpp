#include "equipmentdialog.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

EquipmentDialog::EquipmentDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle("新增设备");
    m_assetNoEdit->setFocus();
}

EquipmentDialog::EquipmentDialog(
    const Equipment& equipment,
    QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setWindowTitle("编辑设备");
    setEquipment(equipment);
}

void EquipmentDialog::setupUi()
{
    setMinimumWidth(520);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_assetNoEdit = new QLineEdit(this);
    m_assetNoEdit->setPlaceholderText("例如：EQ-2026-001");
    formLayout->addRow("资产编号 *：", m_assetNoEdit);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("填写设备名称");
    formLayout->addRow("设备名称 *：", m_nameEdit);

    m_modelEdit = new QLineEdit(this);
    m_modelEdit->setPlaceholderText("选填");
    formLayout->addRow("型号：", m_modelEdit);

    m_brandEdit = new QLineEdit(this);
    m_brandEdit->setPlaceholderText("选填");
    formLayout->addRow("品牌：", m_brandEdit);

    m_purchaseDateEdit = new QDateEdit(QDate::currentDate(), this);
    m_purchaseDateEdit->setCalendarPopup(true);
    m_purchaseDateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow("购置日期：", m_purchaseDateEdit);

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(0.0, 1000000000000.0);
    m_priceSpin->setDecimals(2);
    m_priceSpin->setSuffix(" 元");
    formLayout->addRow("价格：", m_priceSpin);

    m_locationEdit = new QLineEdit(this);
    m_locationEdit->setPlaceholderText("例如：实验楼 302");
    formLayout->addRow("存放位置：", m_locationEdit);

    m_statusCombo = new QComboBox(this);
    for (LabAsset::EquipmentStatus status : LabAsset::allEquipmentStatuses()) {
        m_statusCombo->addItem(
            LabAsset::statusToDisplay(status),
            static_cast<int>(status));
    }
    formLayout->addRow("设备状态：", m_statusCombo);

    m_keeperEdit = new QLineEdit(this);
    m_keeperEdit->setPlaceholderText("填写设备责任人");
    formLayout->addRow("责任人：", m_keeperEdit);

    m_remarkEdit = new QPlainTextEdit(this);
    m_remarkEdit->setPlaceholderText("选填");
    m_remarkEdit->setMinimumHeight(80);
    formLayout->addRow("备注：", m_remarkEdit);

    rootLayout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("保存");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    rootLayout->addWidget(m_buttonBox);

    connect(
        m_buttonBox,
        &QDialogButtonBox::accepted,
        this,
        &EquipmentDialog::accept);
    connect(
        m_buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &EquipmentDialog::reject);
}

void EquipmentDialog::setEquipment(const Equipment& equipment)
{
    m_equipmentId = equipment.id;
    m_assetNoEdit->setText(equipment.assetNo);
    m_nameEdit->setText(equipment.name);
    m_modelEdit->setText(equipment.model);
    m_brandEdit->setText(equipment.brand);

    const QDate purchaseDate =
        QDate::fromString(equipment.purchaseDate, "yyyy-MM-dd");
    if (purchaseDate.isValid()) {
        m_purchaseDateEdit->setDate(purchaseDate);
    }

    m_priceSpin->setValue(equipment.price);
    m_locationEdit->setText(equipment.location);

    const int statusIndex = m_statusCombo->findData(
        static_cast<int>(equipment.status));
    if (statusIndex >= 0) {
        m_statusCombo->setCurrentIndex(statusIndex);
    }

    m_keeperEdit->setText(equipment.keeper);
    m_remarkEdit->setPlainText(equipment.remark);
}

Equipment EquipmentDialog::equipment() const
{
    Equipment result;
    result.id = m_equipmentId;
    result.assetNo = m_assetNoEdit->text().trimmed();
    result.name = m_nameEdit->text().trimmed();
    result.model = m_modelEdit->text().trimmed();
    result.brand = m_brandEdit->text().trimmed();
    result.purchaseDate =
        m_purchaseDateEdit->date().toString("yyyy-MM-dd");
    result.price = m_priceSpin->value();
    result.location = m_locationEdit->text().trimmed();
    result.status = static_cast<LabAsset::EquipmentStatus>(
        m_statusCombo->currentData().toInt());
    result.keeper = m_keeperEdit->text().trimmed();
    result.remark = m_remarkEdit->toPlainText().trimmed();
    return result;
}

void EquipmentDialog::accept()
{
    const QString assetNo = m_assetNoEdit->text().trimmed();
    if (assetNo.isEmpty()) {
        QMessageBox::warning(this, "无法保存", "资产编号不能为空。");
        m_assetNoEdit->setFocus();
        return;
    }

    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "无法保存", "设备名称不能为空。");
        m_nameEdit->setFocus();
        return;
    }

    if (m_repository.assetNoExists(assetNo, m_equipmentId)) {
        QMessageBox::warning(
            this,
            "无法保存",
            "资产编号已存在，请使用其他编号。");
        m_assetNoEdit->setFocus();
        m_assetNoEdit->selectAll();
        return;
    }

    if (!m_repository.lastError().isEmpty()) {
        QMessageBox::warning(
            this,
            "无法校验编号",
            m_repository.lastError());
        return;
    }

    QDialog::accept();
}
