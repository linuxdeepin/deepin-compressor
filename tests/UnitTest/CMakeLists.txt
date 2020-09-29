#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test_output/)



add_subdirectory(deepin-compressor)

add_subdirectory(3rdparty)



