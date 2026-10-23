#include "mainwindow.h"
#include "ui_mainwindow.h" // 注意这里是 ui_ 开头，对应 .ui 文件
#include "exportpage.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 4. 切换到你的统计与导出页面
    connect(ui->btnExport, &QPushButton::clicked, this, [=]() {
        // 1. 切换页面
        ui->stackedWidget->setCurrentWidget(ui->pageExport);

        // 2. 获取该页面的指针，并强制转换为 ExportPage 类型
        ExportPage* exportPage = qobject_cast<ExportPage*>(ui->pageExport);
        if (exportPage) {
            // 3. 调用刷新函数
            exportPage->refreshData();
        }
        });

    // 默认显示设备管理页
    ui->stackedWidget->setCurrentWidget(ui->pageEquipment);

    // 在这里设置窗口标题和初始大小
    setWindowTitle("实验室设备资产管理系统");
    resize(1000, 700);
}

MainWindow::~MainWindow()
{
    delete ui;
}