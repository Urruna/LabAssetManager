add_library(common STATIC
    databasemanager.h
    databasemanager.cpp
    maintenancerecord.h
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC Qt6::Core Qt6::Sql)