#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // 前向声明 UI 命名空间
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT 

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui; // UI 指针
};