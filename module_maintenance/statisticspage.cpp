#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include "statisticspage.h"

StatisticsPage::StatisticsPage(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    refreshStatistics();
}

void StatisticsPage::setupUi()
{
    setObjectName("statisticsPage");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    QHBoxLayout* titleLayout = new QHBoxLayout();

    QLabel* titleLabel = new QLabel("设备状态统计", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    m_refreshButton = new QPushButton("刷新", this);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_refreshButton);
    rootLayout->addLayout(titleLayout);

    m_totalLabel = new QLabel(this);
    rootLayout->addWidget(m_totalLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"设备状态", "数量", "占比"});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rootLayout->addWidget(m_table, 1);

    m_statusLabel = new QLabel(this);
    rootLayout->addWidget(m_statusLabel);

    connect(m_refreshButton,
        &QPushButton::clicked,
        this,
        &StatisticsPage::refreshStatistics);
}

void StatisticsPage::refreshStatistics()
{
    QMap<QString, int> counts = m_repository.countEquipmentByStatus();

    if (!m_repository.lastError().isEmpty()) {
        m_statusLabel->setText("统计失败：" + m_repository.lastError());
        m_totalLabel->setText("设备总数：--");
        m_table->setRowCount(0);
        return;
    }

    QStringList statuses = {
        "available",
        "borrowed",
        "pending_repair",
        "repairing",
        "under_maintenance",
        "scrapped"
    };
    QMap<QString, QString> names;
    names.insert("available", "可用");
    names.insert("borrowed", "借出");
    names.insert("pending_repair", "待维修");
    names.insert("repairing", "维修中");
    names.insert("under_maintenance", "保养中");
    names.insert("scrapped", "报废");

    int total = 0;
    for (int count : counts) {
        total += count;
    }

    m_totalLabel->setText(QString("设备总数：%1").arg(total));
    m_statusLabel->setText("统计范围：全部设备");
    m_table->setRowCount(statuses.size());

    for (int row = 0; row < statuses.size(); ++row) {
        QString status = statuses.at(row);
        int count = counts.value(status, 0);
        double ratio = total > 0 ? count * 100.0 / total : 0.0;

        QTableWidgetItem* nameItem =
            new QTableWidgetItem(names.value(status, status));
        QTableWidgetItem* countItem =
            new QTableWidgetItem(QString::number(count));
        QTableWidgetItem* ratioItem =
            new QTableWidgetItem(QString::number(ratio, 'f', 1) + "%");

        nameItem->setTextAlignment(Qt::AlignCenter);
        countItem->setTextAlignment(Qt::AlignCenter);
        ratioItem->setTextAlignment(Qt::AlignCenter);

        m_table->setItem(row, 0, nameItem);
        m_table->setItem(row, 1, countItem);
        m_table->setItem(row, 2, ratioItem);
    }
}
