# module_export

> 在UI中为主界面：让用户对实验室资产的数据有全局、直观的认识，同时可以导出所有数据。

## 结构

```
module_export 
	CMakeLists.txt
    exportpage.h （页面头文件）
	exportpage.cpp （页面逻辑）
	exportpage.ui （页面 UI）
	csvexporter.h
    csvexporter.cpp 
```

## 功能

1. **数据统计（exportpage）** ：通过 SQL 聚合查询，从数据库获取各个状态的设备数量，并在界面上以 图表（柱状图/饼图）、数据卡片的形式呈现。
2. **数据导出（Export）** ：
   1. **作为公共工具（供 A/B/C 调用）** ：接收他们表格的数据模型（`QAbstractItemModel`），保存为 CSV。每个界面都有自己的查询结果需要导出。
   2. **作为主界面的一键导出** ：在主界面设置导出按钮，主要负责数据库三个表的数据导出（完整的表）。

## 界面

### exportage.ui
