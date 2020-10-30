cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME readonlylibarchiveplugin)
project(${LIB_NAME})

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent)
find_package(KF5Codecs REQUIRED)
find_package(KF5Archive)

#添加dtk库
#find_package(PkgConfig REQUIRED)
#pkg_search_module(DtkWidget REQUIRED dtkwidget)
#include_directories(${DtkWidget_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/../libarchive)
include_directories(${PROJECT_SOURCE_DIR}/../../interface)
include_directories(${PROJECT_SOURCE_DIR}/../../interface/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/../../interface/plugin)
include_directories(${PROJECT_SOURCE_DIR}/../../ChardetDetector)

file(GLOB c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp )
file(GLOB h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h )
file(GLOB json_files RELATIVE ${PROJECT_SOURCE_DIR} *.json)

add_library(${LIB_NAME} SHARED ${c_files} ${h_files} ${json_files})

set(LINK_LIBS
    Qt5::Core
    KF5::Archive
    KF5::Codecs
    compressor-archive
    compressor-interface
    compressor-ChardetDetector
)
target_link_libraries(${LIB_NAME} ${LINK_LIBS})


