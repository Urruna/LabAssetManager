// csvexporter.h
#pragma once
#include <QString>
class QAbstractItemModel;

class CsvExporter {
public:
    // 接收任何数据模型，导出到指定路径
    static bool exportToCsv(QAbstractItemModel* model, const QString& filePath);
};