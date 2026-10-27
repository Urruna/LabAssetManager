# module_maintenance

> 负责维修保养台账的新增、查询和编辑，并联动设备当前状态。状态统计统一由
> `module_export` 的 `ExportPage` 提供。

## 模块结构

```text
module_maintenance
├── CMakeLists.txt
├── maintenancepage.h
├── maintenancepage.cpp
├── maintenancedialog.h
├── maintenancedialog.cpp
├── maintenancepository.h
└── maintenancepository.cpp
```

`MaintenancePage` 的界面由 C++ 代码创建，不需要配套的 `maintenancepage.ui`。

## 已完成功能

1. **新增维修保养记录**
   - 只允许选择当前状态为 `available` 的设备。
   - 维修记录的目标状态为 `pending_repair` 或 `repairing`。
   - 保养记录的目标状态为 `maintaining`。
   - 维保记录插入和设备状态更新在同一个数据库事务中完成。
2. **维修保养台账**
   - 联表显示设备编号、设备名称、日期、类型、说明、费用和经办人。
   - 支持按设备筛选和刷新。
3. **任务状态流转**
   - 维修任务支持 `pending → in_progress → completed`。
   - 保养任务创建后直接进入 `in_progress`。
   - 开始维修时设备由 `pending_repair` 变为 `repairing`。
   - 完成任务后设备恢复为 `available`。
4. **编辑台账记录**
   - 未完成任务可以修改日期、说明、费用和经办人。
   - 已完成记录可以修正设备、类型和其他台账字段。
   - 修改记录不重新驱动设备当前状态。

普通界面不提供删除入口，`MaintenancePository::remove()` 也只允许删除已完成记录。

## 对外接口

集成代码需要依赖以下头文件和公开接口：

```cpp
#include "maintenancepage.h"
```

```cpp
class MaintenancePage : public QWidget
{
    Q_OBJECT

public:
    explicit MaintenancePage(QWidget* parent = nullptr);

public slots:
    void refreshData();
    bool createPendingRepairForDamagedReturn(int equipmentId);

signals:
    void dataChanged();
};
```

约定：

- 页面类名：`MaintenancePage`
- 页面头文件：`maintenancepage.h`
- 刷新槽函数：`refreshData()`
- 数据变化信号：`dataChanged()`

## 集成要求

### 1. CMake 链接

在 `app/CMakeLists.txt` 的 `target_link_libraries` 中增加：

```cmake
module_maintenance
```

修改后应包含：

```cmake
target_link_libraries(LabAssetManager
    PRIVATE
        common
        module_maintenance
        module_export
        Qt6::Widgets
        Qt6::Sql
)
```

### 2. 主窗口接入页面

如果使用 Qt Designer 的自定义控件提升，将 `mainwindow.ui` 中的：

```xml
<widget class="QWidget" name="pageMaintenance"/>
```

改为：

```xml
<widget class="MaintenancePage" name="pageMaintenance"/>
```

并添加：

```xml
<customwidget>
 <class>MaintenancePage</class>
 <extends>QWidget</extends>
 <header>maintenancepage.h</header>
 <container>1</container>
</customwidget>
```

`pageMaintenance` 是页面对象名，`MaintenancePage` 是 C++ 类名，两者不要求同名。

如果不修改 `.ui`，也可以在 `MainWindow` 构造时创建页面，放入
`QStackedWidget` 并替换占位页。

### 3. 导航和刷新

在 `mainwindow.cpp` 中：

```cpp
#include "maintenancepage.h"

connect(ui->btnMaintenance, &QPushButton::clicked, this, [=]() {
    ui->stackedWidget->setCurrentWidget(ui->pageMaintenance);
    ui->pageMaintenance->refreshData();
});
```

维修保养数据发生变化后，需要刷新导出统计页：

```cpp
connect(ui->pageMaintenance,
    &MaintenancePage::dataChanged,
    ui->pageExport,
    &ExportPage::refreshData);
```

借还页发生损坏归还时，由主窗口中转创建待维修任务：

```cpp
connect(ui->pageBorrow,
    &BorrowPage::damagedEquipmentReturned,
    ui->pageMaintenance,
    &MaintenancePage::createPendingRepairForDamagedReturn);
```

设备管理模块不得修改设备状态；设备状态仅由借还和维修保养任务流转。

## 集成验收

- [ ] 主窗口可以显示 `MaintenancePage`，点击“维修记录”能切换页面。
- [ ] 新增对话框只列出 `available` 设备。
- [ ] 新增待维修任务后，设备状态变为 `pending_repair`。
- [ ] 开始维修后，任务变为 `in_progress`，设备变为 `repairing`。
- [ ] 新增保养任务后，任务为 `in_progress`，设备状态变为 `maintaining`。
- [ ] 完成任务后，任务变为 `completed`，设备恢复 `available`。
- [ ] 已借出、维修中或报废的设备不会出现在新增设备下拉框中。
- [ ] 台账能显示设备名称、日期、类型、费用和说明。
- [ ] 编辑台账后设备当前状态不发生变化。
- [ ] 维保页刷新不会崩溃，空台账可以正常显示。
- [ ] 损坏归还后，主窗口能创建待维修任务。
- [ ] 设备编辑不能修改设备状态。
- [ ] 新增、编辑、开始或完成后，各页面同步刷新。
- [ ] 状态统计包含 `pending_repair` 和 `maintaining`。

## 当前状态

`MaintenancePage` 已接入主窗口。最终完成以主窗口集成后的全流程验收结果为准。
