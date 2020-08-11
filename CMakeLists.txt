#1.定义cmake版本
cmake_minimum_required(VERSION 3.8)
#2.设置项目名称
set(EXE_NAME compressor)
project(${EXE_NAME})


#5.1 设置lib输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#5.2 设置可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#include_directories(${PROJECT_SOURCE_DIR}/3rdparty)
add_subdirectory(3rdparty)

#include_directories(${PROJECT_SOURCE_DIR}/deepin-compressor)
add_subdirectory(deepin-compressor)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

message("==============1=========")
message(${CMAKE_INSTALL_PREFIX}/share/deepin/dde-file-manager/oem-menuextensions)
message("==============1=========")

file(GLOB INSTALL_DESKTOP_FILES deepin-compressor/*.desktop)
install(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin/dde-file-manager/oem-menuextensions/)

file(GLOB INSTALL_LIBS_FILES lib/*.so)
install(FILES ${INSTALL_LIBS_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/deepin-compressor/plugins/)

file(GLOB INSTALL_BIN_FILES bin/*)
install(FILES ${INSTALL_BIN_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/deepin-compressor)

file(GLOB INSTALL_QM_FILES translations/*.qm)
install(FILES ${INSTALL_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-compressor/translations)
