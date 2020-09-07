#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME clirarplugin)
project(${EXE_NAME}_test)
#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 REQUIRED Widgets)
include(FindPkgConfig)

include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/clirarplugin)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest/googlemock/include)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/googletest/googletest/include)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/common)


file(GLOB_RECURSE
  c_files
  *.cpp
  ../../../../3rdparty/clirarplugin/*.cpp

  )
file(GLOB_RECURSE
  h_files
  ../../../../3rdparty/clirarplugin/*.h
  )
file(GLOB_RECURSE json_files  ../../../../3rdparty/clirarplugin/*.json)


add_executable(${EXE_NAME}_test  ${h_files} ${c_files}  ${json_files} )

target_link_libraries(${EXE_NAME}_test Qt5::Widgets Qt5::Core )
target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
target_link_libraries(${EXE_NAME}_test ${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME}_test  compressor-interface)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/clirarplugin)
