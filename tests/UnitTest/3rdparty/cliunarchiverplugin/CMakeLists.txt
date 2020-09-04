#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME cliunarchiverplugin)
project(${EXE_NAME}_test)
ADD_COMPILE_OPTIONS(-fno-access-control)
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(Qt5Concurrent)
find_package(Qt5Svg REQUIRED SvgWidgets)
find_package(KF5Codecs REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
include(FindPkgConfig)

include_directories(../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
include_directories(../../../../3rdparty/googletest/googlemock/include)
include_directories(../../../../3rdparty/googletest/googletest/include)
include_directories(../../../../3rdparty/interface)
include_directories(../../../../3rdparty/common)
file(GLOB_RECURSE c_files  *.cpp)

add_executable(${EXE_NAME}_test ${c_files}  )

target_link_libraries(${EXE_NAME}_test Qt5::Widgets Qt5::Core )
target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
