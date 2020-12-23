#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(EXE_NAME ChardetDetector)
project(${EXE_NAME}_test)

#查找依赖关系的包信息
find_package(Qt5Widgets REQUIRED)

#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/ChardetDetector)

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/*.cpp
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/tables/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/*.h
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/tables/*.cpp
    ./*.h
)

file(GLOB_RECURSE tab_files
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/*.tab
    ${3RDPARTYRELATIVEPATH}/ChardetDetector/tables/*.tab
)

#生成可执行文件
add_executable(${EXE_NAME}_test ${c_files} ${h_files} ${tab_files})

#链接动静态库
target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
