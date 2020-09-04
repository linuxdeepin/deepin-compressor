#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME cli7zplugin)
project(${EXE_NAME}_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")



include_directories(../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
include_directories(../../../../3rdparty/googletest/googlemock/include)
include_directories(../../../../3rdparty/googletest/googletest/include)

file(GLOB_RECURSE c_files  *.cpp  )
#add_executable(${EXE_NAME}_test ${c_files}  )

#target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
