#pragma once

#include <QWidget>
#include "statistics_export_global.h"

namespace Ui {
    class StatisticsExportPage;
}

class EXPORT_EXPORT StatisticsExportPage : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsExportPage(QWidget* parent = nullptr);
    ~StatisticsExportPage();

public slots:
    // 约定：主窗口切换过来时，或者数据变动时，会调用这个函数刷新统计数据
    void refreshData();

private slots:
    // UI 按钮的点击事件
    // Qt 自动关联的槽函数命名规范：on_控件名_信号名
    void on_btn_export_equipment_clicked();
    void on_btn_export_borrow_clicked();
    void on_btn_export_maintenance_clicked();

private:
    Ui::StatisticsExportPage* ui;
    void doExport(const QString& defaultFileName, const QString& sqlQuery);
};
