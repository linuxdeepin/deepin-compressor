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
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/archiveinterface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/plugin)
include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/interface/process)


file(GLOB_RECURSE
  c_files
  *.cpp
  )
file(GLOB_RECURSE
  h_files
  )


add_executable(${EXE_NAME}_test  ${h_files} ${c_files}  ${json_files} )

target_link_libraries(${EXE_NAME}_test Qt5::Widgets Qt5::Core )
target_link_libraries(${EXE_NAME}_test ${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME}_test  compressor-interface)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../3rdparty/clirarplugin)
target_link_libraries(${EXE_NAME}_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
