#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME test)
project(${EXE_NAME})

#add_subdirectory(../3rdparty/common common.out)

#add_subdirectory(../3rdparty/interface interface.out)

#add_subdirectory(../3rdparty/ChardetDetector ChardetDetector.out)


add_subdirectory(UnitTest)

#add_subdirectory(ChardetDetector)


