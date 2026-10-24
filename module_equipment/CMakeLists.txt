add_library(module_equipment STATIC
    equipment.h
    equipment.cpp
    equipmentrepository.h
    equipmentrepository.cpp
    equipmentdialog.h
    equipmentdialog.cpp
    equipmentpage.h
    equipmentpage.cpp
    equipmentpage.ui
)

target_include_directories(module_equipment
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(module_equipment
    PUBLIC common Qt6::Widgets Qt6::Sql
)
