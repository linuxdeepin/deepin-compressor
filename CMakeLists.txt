#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME compressor)
project(${EXE_NAME})

#SET(CMAKE_BUILD_TYPE "Debug")  
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

# 添加c++ 11标准支持
set( CMAKE_CXX_FLAGS "-std=c++11" )

#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -Wl,-rpath=./compressor-lib:/usr/lib/deepin-compressor/plugins:/lib") #-Wl,-rpath=./
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wl,-rpath=./compressor-lib:/usr/lib/deepin-compressor/plugins:/lib") #-Wall
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O3 -Wl,-rpath=./compressor-lib:/usr/lib/deepin-compressor/plugins:/lib") #-Wall
add_definitions("-DQT_MESSAGELOGCONTEXT")
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#模糊测试设置 -start
if((${CMAKE_BUILD_TYPE} MATCHES "Debug"))
 #   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
 #   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  #  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
  #  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif()
#模糊测试设置 -end

#3.1 设置lib输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/compressor-lib)
#3.2 设置可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/)

#add_subdirectory(3rdparty/interface)

#add_subdirectory(3rdparty/ChardetDetector)

add_subdirectory(3rdparty) 

add_subdirectory(deepin-compressor)

set(CMAKE_INSTALL_PREFIX /usr)

#file(GLOB INSTALL_DESKTOP_FILES deepin-compressor/*.desktop)
#install(FILES ${INSTALL_DESKTOP_FILES} DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)

#file(GLOB INSTALL_LIBS_FILES ${PROJECT_SOURCE_DIR}/lib/*.so)
#install(FILES ${INSTALL_LIBS_FILES} DESTINATION lib/deepin-compressor/plugins/)

#install(TARGETS compressor-interface LIBRARY DESTINATION lib)

install(TARGETS compressor-interface DESTINATION bin/compressor-lib)
install(TARGETS compressor-ChardetDetector DESTINATION bin/compressor-lib)
install(TARGETS compressor-common DESTINATION bin/compressor-lib)
install(TARGETS compressor-archive DESTINATION bin/compressor-lib)
install(TARGETS compressor-singlefile DESTINATION bin/compressor-lib)

#install(TARGETS compressor-interface DESTINATION lib)
#install(TARGETS compressor-ChardetDetector DESTINATION lib)
#install(TARGETS compressor-common DESTINATION lib)
#install(TARGETS compressor-archive DESTINATION lib)
#install(TARGETS compressor-singlefile DESTINATION lib)

install(TARGETS cli7zplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS clirarplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS cliunarchiverplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS clizipplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS libzipplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS readwritelibarchiveplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS readonlylibarchiveplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS bz2plugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS gzplugin DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS xzplugin DESTINATION lib/deepin-compressor/plugins/)

install(TARGETS compressor-interface DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS compressor-ChardetDetector DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS compressor-common DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS compressor-archive DESTINATION lib/deepin-compressor/plugins/)
install(TARGETS compressor-singlefile DESTINATION lib/deepin-compressor/plugins/)


