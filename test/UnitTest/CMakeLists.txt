#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test_output/)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(include)

add_subdirectory(deepin-compressor)

add_subdirectory(3rdparty)



