project(ChardetDetector)

cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME compressor-ChardetDetector)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
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
file(GLOB_RECURSE tab_files *.tab)

include_directories(${PROJECT_SOURCE_DIR})


add_library(${LIB_NAME} ${c_files} ${h_files}  ${tab_files})
target_include_directories(${LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(${LIB_NAME} Qt5::Widgets)
