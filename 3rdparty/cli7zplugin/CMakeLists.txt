project(cli7zplugin)

cmake_minimum_required(VERSION 3.8)

set(LIB_NAME cli7zplugin)
#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 REQUIRED Widgets)
include(FindPkgConfig)





#set(LIBS_INCLUDE_DIRS 
#	${DtkWidget_LIBRARIES}
#	${DtkCore_LIBRARIES}
#	${DtkGui_LIBRARIES}
#)
set(INTERFACE_LNCLUDE_DIR ${PROJECT_SOURCE_DIR}/../interface)

include_directories(${INTERFACE_LNCLUDE_DIR})
file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h)
file(GLOB_RECURSE json_files  *.json)

add_library(${LIB_NAME} SHARED ${c_files} ${json_files} ${h_files})

#target_link_libraries(${LIB_NAME}  Qt5::Core Qt5::Gui  Qt5::Widgets)
target_link_libraries( ${LIB_NAME} ${LIBS_INCLUDE_DIRS} )
target_link_libraries( ${LIB_NAME} interface)
