# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME interface)
project(${EXE_NAME}_test)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
pkg_search_module(MOUNT REQUIRED mount)


#指定头文件路径
include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${MOUNT_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${KF5Codecs_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/plugin)
include_directories(${PROJECT_SOURCE_DIR}/process)
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/plugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/process)

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ${3RDPARTYRELATIVEPATH}/interface/*.cpp
    ${3RDPARTYRELATIVEPATH}/interface/archiveinterface/*.cpp
    ${3RDPARTYRELATIVEPATH}/interface/plugin/*.cpp
    ${3RDPARTYRELATIVEPATH}/interface/process/*.cpp
    ./*.cpp
    ./archiveinterface/*.cpp
    ./plugin/*.cpp
    ./process/*.cpp
)

file(GLOB_RECURSE h_files
    ${3RDPARTYRELATIVEPATH}/interface/*.h
    ${3RDPARTYRELATIVEPATH}/interface/archiveinterface/*.h
    ${3RDPARTYRELATIVEPATH}/interface/plugin/*.h
    ${3RDPARTYRELATIVEPATH}/interface/process/*.h
    ./*.h
    ./archiveinterface/*.h
    ./plugin/*.h
    ./process/*.h
)

#生成可执行文件
add_executable(${EXE_NAME}_test ${c_files} ${h_files})

#链接动静态库
target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    KF5::Codecs
    ${DtkWidget_LIBRARIES}
    ${MOUNT_LIBRARIES}
    compressor-ChardetDetector
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
