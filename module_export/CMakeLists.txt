add_library(module_export STATIC
    exportpage.h
    exportpage.cpp
    exportpage.ui
    csvexporter.h
    csvexporter.cpp
 "test_main.cpp")

# 把自己的目录暴露出去，这样队友才能 #include "csvexporter.h"
target_include_directories(module_export PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(module_export
    PUBLIC common
    PRIVATE Qt6::Widgets Qt6::Sql
)

add_executable(TestExport test_main.cpp)
target_link_libraries(TestExport PRIVATE module_export Qt6::Widgets Qt6::Sql)
