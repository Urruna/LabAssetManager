add_executable(LabAssetManager
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui   
)
target_link_libraries(LabAssetManager PRIVATE common module_export module_borrow module_maintenance module_equipment Qt6::Widgets Qt6::Sql)

# 从当前电脑找到的 Qt 自动定位 windeployqt，避免写死个人安装路径。
get_filename_component(
    QT6_INSTALL_PREFIX
    "${Qt6_DIR}/../../.."
    ABSOLUTE
)
find_program(
    WINDEPLOYQT_EXECUTABLE
    NAMES windeployqt
    HINTS "${QT6_INSTALL_PREFIX}/bin"
    NO_DEFAULT_PATH
)

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET LabAssetManager POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            $<$<CONFIG:Debug>:--debug>
            $<$<NOT:$<CONFIG:Debug>>:--release>
            --no-translations
            "$<TARGET_FILE:LabAssetManager>"
        COMMENT "正在部署当前 Qt 版本的运行库和插件"
    )
else()
    message(WARNING
        "未找到 windeployqt，程序编译成功，但需要手动部署 Qt 运行库")
endif()
