cmake_minimum_required(VERSION 3.9.5)

project(own-minizip)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)
find_package(ZLIB REQUIRED)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src/archive/api)
include_directories(${BACKEND_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_library(minizipplugin SHARED
    ${PROJECT_SOURCE_DIR}/minizip_plugin.cpp
    ${PROJECT_SOURCE_DIR}/minizip_plugin.h
    ${BACKEND_DIR}/minizip_backend.cpp
    ${BACKEND_DIR}/minizip_backend.h
)

target_link_libraries(minizipplugin
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    compressor-interface
    ZLIB::ZLIB
    minizip
)
