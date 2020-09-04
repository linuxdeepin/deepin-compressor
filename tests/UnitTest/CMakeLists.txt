#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test_output/)

add_subdirectory(deepin-compressor)

add_subdirectory(3rdparty)


#add_subdirectory(deepin-compressor)

##1.定义cmake版本
#cmake_minimum_required(VERSION 3.9.5)
##2.设置项目名称
#set(PROJECT_NAME deepin-compressor)
#set(PROJECT_NAME_TEST
#    ${PROJECT_NAME}_test)
#project(${PROJECT_NAME_TEST})
##配置版本号
##configure_file(environments.h.in environments.h @ONLY)

##SET(CMAKE_BUILD_TYPE "Debug")
##SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
##SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
#ADD_COMPILE_OPTIONS(-fno-access-control)

##3.项目相关设置
##3.1 包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

##3.3 打开全局moc
#set(CMAKE_AUTOMOC ON)
##set(CMAKE_CXX_FLAGS "-std=c++11")
##set(CMAKE_CXX_FLAGS_DEBUG "-O0")
##set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

##demo start
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
##set(CMAKE_CXX_FLAGS "-g -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ldtkwidget -ldtkgui -ldtkcore")#${EXE_NAME}
#set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
##-ldtkwidget -ldtkgui -ldtkcore
#if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
#endif ()


##4 查找依赖关系的包信息
#find_package(PkgConfig REQUIRED)
#find_package(Qt5Core REQUIRED)
#find_package(Qt5DBus REQUIRED)
#find_package(Qt5Gui REQUIRED)
#find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Network REQUIRED)
#find_package(Qt5 REQUIRED Widgets)
#find_package(Qt5Concurrent)
#find_package(Qt5Svg REQUIRED SvgWidgets)
#find_package(KF5Codecs REQUIRED)
#find_package(ZLIB REQUIRED)
#find_package(Qt5LinguistTools REQUIRED)
#include(FindPkgConfig)


#pkg_search_module(DtkWidget REQUIRED dtkwidget)
#include_directories(${DtkWidget_INCLUDE_DIRS})

#pkg_search_module(DtkCore REQUIRED dtkcore)
#include_directories(${DtkCore_INCLUDE_DIRS})

#pkg_search_module(GOBJECT REQUIRED gobject-2.0)
#include_directories(${GOBJECT_INCLUDE_DIRS})

#pkg_search_module(GIO REQUIRED gio-unix-2.0)
#include_directories(${GIO_INCLUDE_DIRS})

#pkg_search_module(DtkGui REQUIRED dtkgui)
#include_directories(${DtkGui_INCLUDE_DIRS})

#set(LIBS_INCLUDE_DIRS
#        ${DtkWidget_LIBRARIES}
#        ${DtkCore_LIBRARIES}
#        ${GOBJECT_LIBRARIES}
#        ${GIO_LIBRARIES}
#        ${DtkGui_LIBRARIES}
#)


##6. 应用interface项目
##interface的h文件
#include_directories(../../3rdparty/interface)
##add_subdirectory(interface)
#message(${PROJECT_SOURCE_DIR})
#message(${PROJECT_SOURCE_DIR})
##7.主程序的h文件
#include_directories(${PROJECT_SOURCE_DIR}/deepin-compressor/source/inc)
#include_directories(${PROJECT_SOURCE_DIR}/deepin-compressor/source/inc/openwithdialog)

##8.主程序的源文件和头文件
#file(GLOB_RECURSE c_files
#    ../../deepin-compressor/source/*.cpp
#    ../../deepin-compressor/source/*.c
#    deepin-compressor/source/src/*.cpp
#    deepin-compressor/source/src/*.c
#    3rdparty/cli7zplugin/*.cpp
#    3rdparty/clirarplugin/*.cpp
#    3rdparty/cliunarchiverplugin/*.cpp
#    3rdparty/common/*.cpp
#    3rdparty/interface/*.cpp
#    3rdparty/libarchive/*.cpp
#    3rdparty/libzipplugin/*.cpp

#    ../../3rdparty/cli7zplugin/*.cpp
#    ../../3rdparty/clirarplugin/*.cpp
#    ../../3rdparty/cliunarchiverplugin/*.cpp
#    ../../3rdparty/common/*.cpp
#    ../../3rdparty/interface/*.cpp
#    ../../ 3rdparty/libarchive/*.cpp
#    ../../ 3rdparty/libzipplugin/*.cpp
#    )
##foreach(i  ${c_files})
##    message(${i})
##endforeach()
#message(${PROJECT_SOURCE_DIR})

#file(GLOB_RECURSE h_files
#    ../../deepin-compressor/source/*.h
#    ../../3rdparty/cli7zplugin/*.h
#    ../../3rdparty/clirarplugin/*.h
#    ../../3rdparty/cliunarchiverplugin/*.h
#    ../../3rdparty/common/*.h
#    ../../3rdparty/interface/*.h
#    ../../ 3rdparty/libarchive/*.h
#    ../../ 3rdparty/libzipplugin/*.h
#    )

##foreach(i  ${h_files})
##    message(${i})
##endforeach()
#include_directories(../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
#include_directories(../3rdparty/googletest/googlemock/include)
#include_directories(../3rdparty/googletest/googletest/include)

#include_directories(../../deepin-compressor/source/inc)
#include_directories(../../deepin-compressor/source/inc/openwithdialog)


#file(GLOB_RECURSE res_files *.qrc)
#set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../translations")
#file( GLOB TS_FILES    ${TS_DIR}/*.ts)
#QT5_ADD_RESOURCES(QRC_MOC ${res_files})

#find_program(LUPDATE_EXECUTABLE lupdate)

###编译时更新.ts .qm
##foreach(_ts_file ${TS_FILES})
##    execute_process(
##        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
##    execute_process(COMMAND lrelease ${_ts_file})
##endforeach()
###lupdate end

#file (GLOB APP_TS_FILES ../translations/*.ts)

#qt5_create_translation(APP_QM_FILES ${APP_TS_FILES} ${APP_QM_FILES})

##INCLUDE(${QT_USE_FILE})
##ADD_DEFINITIONS(${QT_DEFINITIONS})

##9.链接编译的lib
#link_directories(compressor-lib compressor-interface)
#link_directories(compressor-lib compressor-ChardetDetector)
#FILE (GLOB allTestSource
#    "./src/*.cpp" "./src/*.c")

##10.生成可执行程序
#add_executable(${PROJECT_NAME_TEST}  ${c_files} ${h_files} ${QRC_MOC} ${APP_TS_FILES} ${APP_QM_FILES})

## 生成测试可执行程序的依赖库
#target_link_libraries(${PROJECT_NAME_TEST} gmock gmock_main gtest gtest_main)

#set_target_properties(${PROJECT_NAME_TEST} PROPERTIES AUTOMOC TRUE)
#qt5_use_modules(${PROJECT_NAME_TEST}  Core  Widgets  Gui )

#target_include_directories(${PROJECT_NAME_TEST} PRIVATE ../../deepin-compressor/source/inc )
#target_include_directories(${PROJECT_NAME_TEST} PRIVATE ../../deepin-compressor/source/inc/openwithdialog )


#target_link_libraries(${PROJECT_NAME_TEST}  ${LIBS_INCLUDE_DIRS})
##message(${LIBS_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME_TEST}  Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::Svg  KF5::Codecs)


#target_link_libraries(${PROJECT_NAME_TEST}  compressor-interface)
#target_link_libraries(${PROJECT_NAME_TEST}  compressor-ChardetDetector)

