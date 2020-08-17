project(interface)

cmake_minimum_required(VERSION 3.9.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(LIB_NAME compressor-interface)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(QT_MINIMUM_VERSION "5.7.1")


find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)


find_package(Qt5 REQUIRED Widgets)
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

file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h)

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
include_directories("src")

add_library(${LIB_NAME} SHARED  ${h_files} ${c_files}  )
target_link_libraries(${LIB_NAME} Qt5::Widgets Qt5::Core )
target_link_libraries(${LIB_NAME} ${LIBS_INCLUDE_DIRS})
