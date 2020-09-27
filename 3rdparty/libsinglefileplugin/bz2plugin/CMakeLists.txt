
cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME bz2plugin)
project(${LIB_NAME})


#3.项目相关设置
#3.1 包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#3.3 打开全局moc
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_CXX_FLAGS "-std=c++11")
#set(CMAKE_CXX_FLAGS_DEBUG "-O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
 
find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(Qt5Concurrent)
#find_package(Qt5Xml)
find_package(KF5Codecs REQUIRED)
find_package(KF5Archive)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_search_module(DtkGui REQUIRED dtkgui)	
include_directories(${DtkGui_INCLUDE_DIRS})


set(LIBS_INCLUDE_DIRS 
	${DtkWidget_LIBRARIES}
	${DtkCore_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GIO_LIBRARIES}
	${DtkGui_LIBRARIES}
)


include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../)
set(INTERFACE_LNCLUDE_DIR ${PROJECT_SOURCE_DIR}/../../interface)
include_directories(${PROJECT_SOURCE_DIR}/../singlefile)
include_directories(${INTERFACE_LNCLUDE_DIR})


file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h )
file(GLOB_RECURSE json_files  *.json)

add_library(${LIB_NAME} SHARED  ${c_files} ${h_files} ${json_files})


#set_target_properties(${LIB_NAME} PROPERTIES AUTOMOC TRUE)
qt5_use_modules(${LIB_NAME}  Core  Widgets  Gui )
target_include_directories(${LIB_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(${LIB_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::DBus  KF5::Codecs)
target_link_libraries(${LIB_NAME} KF5::Archive)
target_link_libraries(${LIB_NAME} ${LIBS_INCLUDE_DIRS})
target_link_libraries( ${LIB_NAME} compressor-interface)
target_link_libraries( ${LIB_NAME} compressor-singlefile)
