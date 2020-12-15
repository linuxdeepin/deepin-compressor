#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME libzipplugin)
project(${EXE_NAME}_test)
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
    ${libzip_LIBRARIES}

)


include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/archiveinterface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/plugin)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/process)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/ChardetDetector)

include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/libzipplugin)
#include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
#include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest/googlemock/include)
#include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest/googletest/include)


file(GLOB_RECURSE
  c_files
  *.cpp
  )
file(GLOB_RECURSE
  h_files
  )


add_executable(${EXE_NAME}_test  ${h_files} ${c_files}  ${json_files} )

target_link_libraries(${EXE_NAME}_test Qt5::Widgets Qt5::Core KF5::Codecs)
#target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
target_link_libraries(${EXE_NAME}_test ${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME}_test  compressor-ChardetDetector)
target_link_libraries(${EXE_NAME}_test  compressor-interface)
target_link_libraries(${EXE_NAME}_test -lzip)
target_link_libraries(${EXE_NAME}_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
