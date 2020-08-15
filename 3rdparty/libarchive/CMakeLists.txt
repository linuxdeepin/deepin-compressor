#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)
#2.设置项目名称
set(EXE_NAME libarchive)
project(${EXE_NAME})

add_subdirectory(libarchive)

add_subdirectory(readwritelibarchiveplugin)

add_subdirectory(readonlylibarchiveplugin)






