#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME readwritelibarchiveplugin)
project(${EXE_NAME}_test)
#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(KF5Codecs REQUIRED)
find_package(LibArchive)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_search_module(DtkGui REQUIRED dtkgui)
include_directories(${DtkGui_INCLUDE_DIRS})

pkg_search_module(LibArchive REQUIRED libarchive)
include_directories(${LibArchive_INCLUDE_DIRS})

set(LIBS_INCLUDE_DIRS
        ${DtkWidget_LIBRARIES}
        ${DtkCore_LIBRARIES}
        ${GOBJECT_LIBRARIES}
        ${GIO_LIBRARIES}
        ${DtkGui_LIBRARIES}
        ${LibArchive_LIBRARIES}
)
include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/common)
include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/interface)
include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/ChardetDetector)
include_directories(../../../../../3rdparty/libarchive)
include_directories(../../../../../3rdparty/libarchive/libarchive)
include_directories( ../../../../../3rdparty/libarchive/readonlylibarchiveplugin)
include_directories(../../../../../3rdparty/libarchive/readwritelibarchiveplugin)

#include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/googletest)                           #一下三行添加包含以便测试用例代码的编写
#include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/googletest/googlemock/include)
#include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/googletest/googletest/include)



file(GLOB_RECURSE
  c_files
  *.cpp
  ../../../../../3rdparty/libarchive/libarchive/*.cpp
  ../../../../../3rdparty/libarchive/readwritelibarchiveplugin/*.cpp
  )
file(GLOB_RECURSE
  h_files
  ../../../../../3rdparty/libarchive/libarchive/*.h
  ../../../../../3rdparty/libarchive/readwritelibarchiveplugin/*.h
  )
file(GLOB_RECURSE
    json_files
    ../../../../../3rdparty/libarchive/libarchive/*.json
    ../../../../../3rdparty/libarchive/readwritelibarchiveplugin/*.json
    )


add_executable(${EXE_NAME}_test  ${h_files} ${c_files}  ${json_files} )

target_link_libraries(${EXE_NAME}_test  Qt5::Widgets Qt5::Core KF5::Codecs)
#target_link_libraries(${EXE_NAME}_test gmock gmock_main gtest gtest_main)
target_link_libraries(${EXE_NAME}_test ${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME}_test  compressor-interface)
target_link_libraries(${EXE_NAME}_test  compressor-common)
target_link_libraries(${EXE_NAME}_test  compressor-ChardetDetector)


target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/common)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/interface)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/libarchive)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/libarchive/libarchive)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/libarchive/readonlylibarchiveplugin)
target_include_directories(${EXE_NAME}_test PRIVATE ../../../../../3rdparty/libarchive/readwritelibarchiveplugin)
target_link_libraries(${EXE_NAME}_test
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
