add_executable(LabAssetManager
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui   
)
target_link_libraries(LabAssetManager PRIVATE common module_export Qt6::Widgets Qt6::Sql)

# 将所需dll复制到exe所在目录
add_custom_command(TARGET LabAssetManager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/bin/Qt6Cored.dll"
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/bin/Qt6Widgetsd.dll"
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/bin/Qt6Sqld.dll"
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/bin/Qt6Guid.dll"
        $<TARGET_FILE_DIR:LabAssetManager>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/plugins/platforms"
        "$<TARGET_FILE_DIR:LabAssetManager>/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "D:/AppGallery/Qt/6.0.1/msvc2019_64/plugins/sqldrivers"
        "$<TARGET_FILE_DIR:LabAssetManager>/sqldrivers"
)