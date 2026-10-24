add_library(module_maintenance STATIC
    maintenancepage.h
    maintenancepage.cpp
    maintenancedialog.h
    maintenancedialog.cpp
    maintenancepository.h
    maintenancepository.cpp
)

target_include_directories(module_maintenance
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(module_maintenance
    PUBLIC common Qt6::Widgets Qt6::Sql
)
