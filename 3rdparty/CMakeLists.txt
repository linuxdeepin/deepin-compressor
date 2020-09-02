#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME 3rdparty)
project(${EXE_NAME})

add_subdirectory(interface)

add_subdirectory(ChardetDetector)

add_subdirectory(common)

add_subdirectory(cli7zplugin)

add_subdirectory(clirarplugin)

add_subdirectory(cliunarchiverplugin)

add_subdirectory(clizipplugin)

add_subdirectory(libzipplugin)

add_subdirectory(libarchive)

add_subdirectory(libsinglefileplugin)





