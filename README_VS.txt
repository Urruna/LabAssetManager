LabAssetManager Visual Studio / Qt 工程说明

一、解决方案

1. common/common.sln
2. module_equipment/module_equipment.sln
3. module_borrow/module_borrow.sln
4. module_maintenance/module_maintenance.sln
5. module_statistics_export/module_statistics_export.sln
6. app/LabAssetManager.sln

每个模块解决方案包含模块 DLL 项目和独立测试程序。
最终 app/LabAssetManager.sln 链接 common 和四个模块 DLL。

二、构建顺序

1. common/common.sln
2. 四个 module_*.sln
3. app/LabAssetManager.sln
4. 在 Visual Studio 中按 F5 运行 LabAssetManager

三、属性和 Qt 配置

commonlibdll 中的属性表负责：

- QtDebug.props / QtRelease.props：Qt 头文件、库和 Debug/Release Qt 库。
- ModuleDebug.props / ModuleRelease.props：模块链接 common。
- TestDebug.props / TestRelease.props：测试程序链接 common 和当前模块。
- AppDebug.props / AppRelease.props：最终程序链接全部模块。

全部 11 个 .vcxproj 已导入对应属性表。

测试程序和最终 APP 通过属性表中的 LocalDebuggerEnvironment 查找 Qt 与
业务 DLL，不在开发输出目录重复复制整套 Qt 运行库。这与老师示例使用
Qt VS Tools 调试环境的组织方式一致。

Qt 版本：
C:/Qt/6.5.3/msvc2019_64

四、commonlibdll

- <module>/include：模块公共头文件
- <module>/lib：DLL 导入库
- <module>/bin：模块 DLL 和独立测试程序

Debug DLL 使用 d 后缀，Release DLL 使用无后缀名称。

五、模块导出宏

- common/common_global.h
- module_equipment/equipment_global.h
- module_borrow/borrow_global.h
- module_maintenance/maintenance_global.h
- module_statistics_export/statistics_export_global.h

六、测试项目

- testCommon
- testEquipment
- testBorrow
- testMaintenance
- testStatisticsExport

测试项目用于单独运行和检查对应模块页面，不参与最终 APP。

七、运行和发布

开发阶段在 Visual Studio 中按 F5 运行。bin 目录只保留最终 APP 可执行文件，
Qt 和业务 DLL 由 AppDebug.props / AppRelease.props 的调试 PATH 提供。

如果需要脱离 Visual Studio 双击运行，应另外生成 publish 目录，并将 Qt
运行库、业务 DLL 和插件复制到该发布目录，避免把部署文件混入开发目录。
