cmake_minimum_required(VERSION 3.16)
project(LabAssetManager VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/AppGallery/Qt/6.0.1/msvc2019_64" CACHE PATH "Qt install prefix")
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

add_subdirectory(common)
add_subdirectory(module_maintenance)
add_subdirectory(app)