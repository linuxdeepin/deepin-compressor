#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)
#2.设置项目名称
set(EXE_NAME 3rdparty)
project(${EXE_NAME})


add_subdirectory(ChardetDetector)

add_subdirectory(cli7zplugin)

#include_directories(clirarplugin)
add_subdirectory(clirarplugin)

add_subdirectory(cliunarchiverplugin)

add_subdirectory(clizipplugin)

add_subdirectory(libarchive/readwritelibarchiveplugin)

add_subdirectory(libarchive/readonlylibarchiveplugin)

add_subdirectory(libsinglefileplugin/bz2plugin)

add_subdirectory(libsinglefileplugin/gzplugin)

add_subdirectory(libsinglefileplugin/xzplugin)


