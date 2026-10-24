#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "maintenancedialog.h"

MaintenanceDialog::MaintenanceDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    loadEquipment();
    updateStatusOptions();
}

MaintenanceDialog::MaintenanceDialog(
    const MaintenanceRecord& record,
    QWidget* parent)
    : QDialog(parent),
      m_editing(true)
{
    setupUi();
    loadEquipment();
    selectRecord(record);
    setWindowTitle("编辑维修保养记录");
}

void MaintenanceDialog::setupUi()
{
    setWindowTitle("新增维修保养记录");
    setMinimumWidth(480);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    m_equipmentCombo = new QComboBox(this);
    m_equipmentCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_equipmentCombo->setMinimumContentsLength(24);
    formLayout->addRow("设备：", m_equipmentCombo);

    m_dateEdit = new QDateEdit(QDate::currentDate(), this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow("日期：", m_dateEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("维修", "repair");
    m_typeCombo->addItem("保养", "maintain");
    formLayout->addRow("类型：", m_typeCombo);

    m_statusLabel = new QLabel("设备状态：", this);
    m_statusCombo = new QComboBox(this);
    formLayout->addRow(m_statusLabel, m_statusCombo);

    m_costSpin = new QDoubleSpinBox(this);
    m_costSpin->setRange(0.0, 99999999.0);
    m_costSpin->setDecimals(2);
    m_costSpin->setSuffix(" 元");
    formLayout->addRow("费用：", m_costSpin);

    m_descriptionEdit = new QPlainTextEdit(this);
    m_descriptionEdit->setPlaceholderText("填写故障情况或保养内容");
    m_descriptionEdit->setMinimumHeight(90);
    formLayout->addRow("说明：", m_descriptionEdit);

    m_operatorEdit = new QLineEdit(this);
    m_operatorEdit->setPlaceholderText("填写经办人");
    formLayout->addRow("经办人：", m_operatorEdit);

    rootLayout->addLayout(formLayout);

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);
    rootLayout->addWidget(m_buttonBox);

    connect(m_typeCombo,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        [this](int) {
            updateStatusOptions();
        });
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &MaintenanceDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &MaintenanceDialog::reject);
}

void MaintenanceDialog::loadEquipment()
{
    m_equipmentCombo->clear();

    // 新增记录只允许选择可用设备；编辑历史台账时显示全部设备。
    QList<MaintenanceEquipmentOption> options =
        m_repository.findEquipmentOptions(!m_editing);

    if (!m_repository.lastError().isEmpty()) {
        m_equipmentCombo->addItem("设备加载失败", -1);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    for (const MaintenanceEquipmentOption& option : options) {
        QString text = option.assetNo + " - " + option.name;
        m_equipmentCombo->addItem(text, option.id);
    }

    if (m_equipmentCombo->count() == 0) {
        m_equipmentCombo->addItem("暂无可登记设备", -1);
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void MaintenanceDialog::updateStatusOptions()
{
    m_statusCombo->clear();

    QString type = m_typeCombo->currentData().toString();
    if (type == "repair") {
        m_statusCombo->addItem("待维修", "pending_repair");
        m_statusCombo->addItem("维修中", "repairing");
    }
    else {
        m_statusCombo->addItem("保养中", "under_maintenance");
    }

    // 编辑台账只修正历史数据，不重新驱动设备当前状态。
    m_statusLabel->setVisible(!m_editing);
    m_statusCombo->setVisible(!m_editing);
}

void MaintenanceDialog::selectRecord(const MaintenanceRecord& record)
{
    m_recordId = record.id;

    int equipmentIndex = m_equipmentCombo->findData(record.equipmentId);
    if (equipmentIndex >= 0) {
        m_equipmentCombo->setCurrentIndex(equipmentIndex);
    }

    QDate maintenanceDate =
        QDate::fromString(record.maintenanceDate, "yyyy-MM-dd");
    if (maintenanceDate.isValid()) {
        m_dateEdit->setDate(maintenanceDate);
    }

    int typeIndex = m_typeCombo->findData(record.type);
    if (typeIndex >= 0) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }

    m_costSpin->setValue(record.cost);
    m_descriptionEdit->setPlainText(record.description);
    m_operatorEdit->setText(record.operatorName);

    updateStatusOptions();
}

MaintenanceRecord MaintenanceDialog::record() const
{
    MaintenanceRecord result;
    result.id = m_recordId;
    result.equipmentId = m_equipmentCombo->currentData().toInt();
    result.maintenanceDate = m_dateEdit->date().toString("yyyy-MM-dd");
    result.type = m_typeCombo->currentData().toString();
    result.description = m_descriptionEdit->toPlainText().trimmed();
    result.cost = m_costSpin->value();
    result.operatorName = m_operatorEdit->text().trimmed();
    return result;
}

QString MaintenanceDialog::equipmentStatus() const
{
    return m_statusCombo->currentData().toString();
}

void MaintenanceDialog::accept()
{
    if (m_equipmentCombo->currentData().toInt() <= 0) {
        QMessageBox::warning(this, "无法保存", "请选择一台可用设备。");
        return;
    }

    if (m_descriptionEdit->toPlainText().trimmed().isEmpty()) {
        QMessageBox::warning(this, "无法保存", "请填写故障情况或保养内容。");
        return;
    }

    QDialog::accept();
}
