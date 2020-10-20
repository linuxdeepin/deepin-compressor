cmake_minimum_required(VERSION 3.9.5)
set(LIB_NAME compressor-archive)
project(${LIB_NAME})


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_MINIMUM_VERSION "5.7.1")


find_package(Qt5Core REQUIRED)
# 添加libarchive.so
find_package(LibArchive REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)

# Compile definitions for TerminalWidget
# So we can use QT_VERSION_CHECK
set(TERMINALWIDGET_VERSION_MAJOR "0")
set(TERMINALWIDGET_VERSION_MINOR "14")
set(TERMINALWIDGET_VERSION_PATCH "1")

include_directories(${PROJECT_SOURCE_DIR}/../../interface)
include_directories(${PROJECT_SOURCE_DIR}/../../ChardetDetector)

file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h)

add_library(${LIB_NAME} STATIC ${h_files} ${c_files})

set(LINK_LIBS
    Qt5::Core
    ${GOBJECT_LIBRARIES}
    ${LibArchive_LIBRARIES}
    compressor-interface
    compressor-ChardetDetector
)
target_link_libraries(${LIB_NAME} ${LINK_LIBS})

