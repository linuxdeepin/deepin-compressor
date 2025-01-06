# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(PROJECT_NAME_TEST deepin-compressor_test)
project(${PROJECT_NAME_TEST})
#配置版本号
#configure_file(environments.h.in environments.h @ONLY)

#ADD_COMPILE_OPTIONS(-fno-access-control) #访问私有方法
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-access-control")

#if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
#endif ()

set(SRCRELATIVEPATH ../../../src) #相对于源码路径
set(LIBZIPPLUGINPATH ../../../3rdparty/libzipplugin) #相对于源码路径

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
include(FindPkgConfig)

set(qt_required_components Widgets Concurrent Test)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND qt_required_components Core5Compat)
endif()

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS ${qt_required_components})
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)

set(LINK_LIBS
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Concurrent
    Qt${QT_DESIRED_VERSION}::Test
    Dtk${DTK_VERSION_MAJOR}::Widget
    KF${KF_VERSION_MAJOR}::Codecs
)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND LINK_LIBS Qt${QT_DESIRED_VERSION}::Core5Compat)
endif()

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})



 
#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})

#主程序的h文件
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/archivemanager)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/common)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/dialog)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/dialog/openwithdialog)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/page)
include_directories(${PROJECT_SOURCE_DIR}/${SRCRELATIVEPATH}/source/tree)
include_directories(${PROJECT_SOURCE_DIR}/${LIBZIPPLUGINPATH})

#源文件、头文件和资源文件
include_directories(${PROJECT_SOURCE_DIR}/source/common)
file(GLOB_RECURSE c_files
    ${SRCRELATIVEPATH}/source/*.cpp
    ${SRCRELATIVEPATH}/source/archivemanager/*.cpp
    ${SRCRELATIVEPATH}/source/common/*.cpp
    ${SRCRELATIVEPATH}/source/dialog/*.cpp
    ${SRCRELATIVEPATH}/source/dialog/openwithdialog/*.cpp
    ${SRCRELATIVEPATH}/source/page/*.cpp
    ${SRCRELATIVEPATH}/source/tree/*.cpp
    ${LIBZIPPLUGINPATH}/*.cpp

    ./source/*.cpp
    ./source/archivemanager/*.cpp
    ./source/common/*.cpp
    ./source/dialog/*.cpp
    ./source/dialog/openwithdialog/*.cpp
    ./source/page/*.cpp
    ./source/tree/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ${SRCRELATIVEPATH}/source/*.h
    ${SRCRELATIVEPATH}/source/archivemanager/*.h
    ${SRCRELATIVEPATH}/source/common/*.h
    ${SRCRELATIVEPATH}/source/dialog/*.h
    ${SRCRELATIVEPATH}/source/dialog/openwithdialog/*.h
    ${SRCRELATIVEPATH}/source/page/*.h
    ${SRCRELATIVEPATH}/source/tree/*.h
    ${LIBZIPPLUGINPATH}/*.h

    ./source/*.h
    ./source/archivemanager/*.h
    ./source/common/*.h
    ./source/dialog/*.h
    ./source/dialog/openwithdialog/*.h
    ./source/page/*.h
    ./source/tree/*.h
    ./*.h
)

file(GLOB_RECURSE res_files ${SRCRELATIVEPATH}/*.qrc)
# 根据Qt版本选择合适的命令
if (QT_VERSION VERSION_GREATER_EQUAL "6.0")
    qt_add_resources(QRC_MOC ${res_files})
else()
    qt5_add_resources(QRC_MOC ${res_files})
endif()

#INCLUDE(${QT_USE_FILE})
#ADD_DEFINITIONS(${QT_DEFINITIONS})

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

FILE (GLOB allTestSource
    "./src/*.cpp" "./src/*.c")

#10.生成可执行程序
add_executable(${PROJECT_NAME_TEST}  ${c_files} ${h_files} ${QRC_MOC} ${APP_TS_FILES} ${APP_QM_FILES})

# 生成测试可执行程序的依赖库
#target_link_libraries(${PROJECT_NAME_TEST} gmock gmock_main gtest gtest_main)

set_target_properties(${PROJECT_NAME_TEST} PROPERTIES AUTOMOC TRUE)

#链接动静态库
target_link_libraries(${PROJECT_NAME_TEST}
    ${LINK_LIBS}
    ${GOBJECT_LIBRARIES}
    ${GIO_LIBRARIES}
    compressor-interface
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    zip
)

