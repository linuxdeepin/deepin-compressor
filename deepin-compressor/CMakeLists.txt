#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)
#2.设置项目名称
set(EXE_NAME deepin-compressor)
project(${EXE_NAME})

#3.项目相关设置
#3.1 包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#3.2 设置cmake类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
 

#4 查找依赖关系的包信息
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

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_search_module(DISOMASTER REQUIRED disomaster)
include_directories(${DISOMASTER_INCLUDE_DIRS})

pkg_search_module(QTUDISKS2 REQUIRED udisks2-qt5)
include_directories(${QTUDISKS2_INCLUDE_DIRS})

pkg_search_module(QTGSetting REQUIRED gsettings-qt)
include_directories(${QTGSetting_INCLUDE_DIRS})

pkg_search_module(LIBSECRET REQUIRED libsecret-1)	
include_directories(${LIBSECRET_INCLUDE_DIRS})
 
pkg_check_modules(ATSPI2 REQUIRED atspi-2)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)


pkg_search_module(DtkGui REQUIRED dtkgui)	
include_directories(${DtkGui_INCLUDE_DIRS})


set(LIBS_INCLUDE_DIRS 
	${DtkWidget_LIBRARIES}
	${DtkCore_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GIO_LIBRARIES}
	${DISOMASTER_LIBRARIES}
	${QTUDISKS2_LIBRARIES}
	${QTGSetting_LIBRARIES}
	${LIBSECRET_LIBRARIES}
	${DtkGui_LIBRARIES}
)


 
#5设置库文件输出目录

#5.1 设置lib输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#5.2 设置可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
 
 
#6. 应用interface项目
#interface的h文件
include_directories(${PROJECT_SOURCE_DIR}/interface)
add_subdirectory(interface)

#7.主程序的h文件
include_directories(${PROJECT_SOURCE_DIR}/source/inc)
include_directories(${PROJECT_SOURCE_DIR}/source/inc/openwithdialog)

#8.主程序的源文件和头文件
file(GLOB_RECURSE c_files  source/*.cpp source/*.c main.cpp)
file(GLOB_RECURSE h_files  source/inc/*.h source/inc/openwithdialog/*.h )
file(GLOB_RECURSE res_files *.qrc)

message("============================")
foreach(f ${res_files})
message(${f})
Endforeach(f) 
message("============================")

#QT5_WRAP_CPP(HEADER_MOC ${h_files})
QT5_ADD_RESOURCES(QRC_MOC ${res_files})

#INCLUDE(${QT_USE_FILE})
#ADD_DEFINITIONS(${QT_DEFINITIONS})

#9.链接编译的lib
link_directories(${PROJECT_SOURCE_DIR}/lib)

#10.生成可执行程序
add_executable(${EXE_NAME}  ${c_files} ${h_files} ${QRC_MOC})



set_target_properties(${EXE_NAME} PROPERTIES AUTOMOC TRUE)
qt5_use_modules(${EXE_NAME}  Core  Widgets  Xml Gui )

target_include_directories(${EXE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source/inc )
target_include_directories(${EXE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source/inc/openwithdialog )

target_link_libraries(${EXE_NAME}  ${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::DBus Qt5::Svg  KF5::Codecs)
target_link_libraries(${EXE_NAME} interface)