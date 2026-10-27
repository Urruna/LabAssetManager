# Included by the team's Qt6 root project after common.
add_library(module_borrow STATIC
    borrowpage.h
    borrowpage.cpp
    borrowpage.ui
    borrowdialog.h
    borrowdialog.cpp
    borrowdialog.ui
    returndialog.h
    returndialog.cpp
    returndialog.ui
)

target_include_directories(module_borrow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(module_borrow PUBLIC common Qt6::Widgets Qt6::Sql)