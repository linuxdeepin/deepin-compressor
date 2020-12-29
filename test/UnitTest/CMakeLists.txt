#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称

#访问私有方法 -fno-access-control
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-access-control")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test_output/)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -Wl,-rpath=${LIBRARY_OUTPUT_PATH}:/usr/lib/deepin-compressor/plugins")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1,-rpath=${LIBRARY_OUTPUT_PATH}:/usr/lib/deepin-compressor/plugins")
endif()

#查找依赖关系的包信息
find_package(GTest REQUIRED)

#指定头文件路径
include_directories(include) #提供stub.h、addr_pri.h
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${HFILES_OUTPUT_PATH})

add_subdirectory(src)

add_subdirectory(3rdparty)
