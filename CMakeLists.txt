#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)

#2.设置项目名称
set(EXE_NAME compressor)
project(${EXE_NAME})

SET(CMAKE_BUILD_TYPE "Debug")  
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

# 添加c++ 11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11" )

#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


#3.1 设置lib输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/compressor-lib)
#3.2 设置可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

add_subdirectory(3rdparty/interface)

add_subdirectory(3rdparty/ChardetDetector)


add_subdirectory(3rdparty) 

add_subdirectory(deepin-compressor)

set(CMAKE_INSTALL_PREFIX /usr)

#file(GLOB INSTALL_DESKTOP_FILES deepin-compressor/*.desktop)
#install(FILES ${INSTALL_DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin/dde-file-manager/oem-menuextensions/)

#file(GLOB INSTALL_LIBS_FILES ${PROJECT_SOURCE_DIR}/lib/*.so)
#install(FILES ${INSTALL_LIBS_FILES} DESTINATION /usr/lib/deepin-compressor/plugins/)

install(TARGETS interface DESTINATION /usr/lib/compressor-lib)
install(TARGETS ChardetDetector DESTINATION /usr/lib/compressor-lib)
install(TARGETS interface DESTINATION /usr/lib)
install(TARGETS ChardetDetector DESTINATION /usr/lib)
install(TARGETS cli7zplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS clirarplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS cliunarchiverplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS clizipplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS libzipplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS readwritelibarchiveplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS readonlylibarchiveplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS bz2plugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS gzplugin DESTINATION /usr/lib/deepin-compressor/plugins/)
install(TARGETS xzplugin DESTINATION /usr/lib/deepin-compressor/plugins/)





