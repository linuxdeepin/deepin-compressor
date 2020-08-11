#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)

#2.设置项目名称
set(LIB_NAME libzipplugin)


find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(Qt5Concurrent)
find_package(Qt5Xml)
find_package(Qt5Svg REQUIRED SvgWidgets)
find_package(KF5Codecs REQUIRED)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})


file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h)
include_directories(${PROJECT_SOURCE_DIR}/../../deepin-compressor/interface)
include_directories(${PROJECT_SOURCE_DIR}/../ChardetDetector)

set( CASSDK_DIR ${PROJECT_SOURCE_DIR}/../../deepin-compressor/)
add_subdirectory(${CASSDK_DIR}/interface interface.out)

add_library(${LIB_NAME} SHARED  ${h_files} ${c_files}  )
target_link_libraries(${LIB_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::DBus Qt5::Svg  KF5::Codecs)