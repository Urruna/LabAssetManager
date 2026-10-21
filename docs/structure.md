```
LabAssetManager/
├── CMakeLists.txt                  # 顶层
├── CMakePresets.json               # 锁定生成器/工具集/Qt路径
├── .gitignore
├── README.md                       # 环境要求、分工、构建步骤
│
├── docs/
│   ├── schema.sql                  # 数据库唯一权威版本
│   └── design.md                   # 架构说明、模块边界
│
├── common/                         # 公共接口（A 维护，全员可提 PR）
│   ├── CMakeLists.txt
│   ├── databasemanager.h
│   ├── databasemanager.cpp
│   ├── equipment.h                 # Equipment 数据结构 + getter/setter
│   ├── equipment.cpp
│   ├── enums.h                     # 状态、维保类型等常量与中文映射
│   └── utils.h / utils.cpp         # 日期格式化等小工具
│
├── module_equipment/               # A：设备管理
│   ├── CMakeLists.txt
│   ├── equipmentpage.h / .cpp      # 设备管理页（QWidget）
│   ├── equipmentdialog.h / .cpp    # 新增/编辑对话框
│   └── equipmentrepository.h / .cpp  # 设备表的 CRUD
│
├── module_borrow/                  # B：借还管理
│   ├── CMakeLists.txt
│   ├── borrowpage.h / .cpp
│   ├── borrowdialog.h / .cpp
│   └── borrowrepository.h / .cpp
│
├── module_maintenance/             # C：维保 + 统计
│   ├── CMakeLists.txt
│   ├── maintenancepage.h / .cpp
│   ├── maintenancedialog.h / .cpp
│   ├── maintenancepository.h / .cpp
│   └── statisticspage.h / .cpp     # 状态统计
│
├── module_export/                  # D：导出 + 集成辅助
│   ├── CMakeLists.txt
│   ├── exporter.h / .cpp           # CSV 导出
│   └── exportdialog.h / .cpp
│
├── app/                            # 主程序
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── mainwindow.h / .cpp         # 导航 + QStackedWidget
│   └── mainwindow.ui
│
└── tests/                          # 可选：B 的纯逻辑单测
    ├── CMakeLists.txt
    └── test_borrow_logic.cpp
```
