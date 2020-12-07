#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(LIB_NAME minizipplugin)

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
find_package(ZLIB REQUIRED)

include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})

if (PKG_CONFIG_FOUND)
#       PKG_CHECK_MODULES(UNZIP minizip)
endif (PKG_CONFIG_FOUND)

#message(${unzip_LIBRARIES})

set(LIBS_INCLUDE_DIRS 
	${DtkWidget_LIBRARIES}
	${DtkCore_LIBRARIES}
  #${UNZIP_LIBRARIES}
)
#message(${unzip_LIBRARIES}) 

include_directories(../ChardetDetector/)
include_directories(../interface/)
include_directories(../interface/archiveinterface)
include_directories(../interface/plugin)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE c_files  *.cpp )
file(GLOB_RECURSE h_files  *.h )
file(GLOB_RECURSE json_files  *.json)

add_library(${LIB_NAME} SHARED ${c_files} ${json_files} ${h_files})
target_link_libraries(${LIB_NAME} Qt5::Widgets Qt5::Core KF5::Codecs)
target_link_libraries( ${LIB_NAME} ${LIBS_INCLUDE_DIRS})
target_link_libraries( ${LIB_NAME} compressor-interface)
target_link_libraries( ${LIB_NAME} compressor-ChardetDetector)
target_link_libraries( ${LIB_NAME} -lminizip)
