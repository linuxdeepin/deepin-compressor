#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME interface)
project(${EXE_NAME}_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
ADD_COMPILE_OPTIONS(-fno-access-control)
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
include(FindPkgConfig)


pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(DtkGui REQUIRED dtkgui)
pkg_search_module(DtkCore REQUIRED dtkcore)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)


# Compile definitions for TerminalWidget
# So we can use QT_VERSION_CHECK
set(TERMINALWIDGET_VERSION_MAJOR "0")
set(TERMINALWIDGET_VERSION_MINOR "14")
set(TERMINALWIDGET_VERSION_PATCH "1")


include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})
set(LIBS_INCLUDE_DIRS
  ${DtkWidget_LIBRARIES}
  ${DtkCore_LIBRARIES}
  ${DtkCore_LIBRARIES}
)

#include_directories(../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
#include_directories(../../../../3rdparty/googletest/googlemock/include)
#include_directories(../../../../3rdparty/googletest/googletest/include)
include_directories(../../../../3rdparty/interface)
include_directories(../../../../3rdparty/common)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate
    Qt5::Network

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${ATSPI2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)

include_directories(${LIB_NAME} PUBLIC ${Qt5Widgets_LIBRARIES}
                                              ${Qt5DBus_LIBRARIES}
                                              ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                              ${PROJECT_BINARY_DIR}
                                              ${DtkWidget_INCLUDE_DIRS}
                                              ${DtkCore_LIBRARIES}
                                              ${DtkGUI_INCLUDE_DIRS}
                                              ${ATSPI2_INCLUDE_DIRS}
                                              ${GOBJECT_INCLUDE_DIRS})
file(GLOB_RECURSE
    c_files
    *.cpp
    ../../../../3rdparty/interface/*.cpp
    )
file(GLOB_RECURSE
    h_files
    ../../../../3rdparty/interface/*.h
    )

add_executable(${EXE_NAME}_test ${c_files} ${h_files}  )

target_link_libraries(${EXE_NAME}_test Qt5::Widgets Qt5::Core )
#target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
target_link_libraries(${EXE_NAME}_test ${LIBS_INCLUDE_DIRS})

target_include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface )
target_link_libraries(${EXE_NAME}_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
