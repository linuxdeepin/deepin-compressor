#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
project(libsinglefileplugin)

add_subdirectory(singlefile)

add_subdirectory(bz2plugin)

add_subdirectory(gzplugin)

add_subdirectory(xzplugin)
