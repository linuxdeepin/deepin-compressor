#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME common)
project(${EXE_NAME}_test)
ADD_COMPILE_OPTIONS(-fno-access-control)
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")

find_package(Qt5Core REQUIRED)

find_package(KF5Codecs REQUIRED)


include_directories(../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
include_directories(../../../../3rdparty/googletest/googlemock/include)
include_directories(../../../../3rdparty/googletest/googletest/include)

file(GLOB_RECURSE
    c_files
    *.cpp
    ../../../../3rdparty/common/*.cpp
    )
file(GLOB_RECURSE
    h_files
    ../../../../3rdparty/common/*.h
    )

add_executable(${EXE_NAME}_test ${c_files} ${h_files})

target_link_libraries(${EXE_NAME}_test
    Qt5::Core
    KF5::Codecs
    compressor-common
)

target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)

target_include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/common)
