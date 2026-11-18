cmake_minimum_required(VERSION 3.9.5)

project(own-libzip)

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)
find_package(ZLIB REQUIRED)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src/archive/api)
include_directories(${BACKEND_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_library(libzipplugin SHARED
    ${PROJECT_SOURCE_DIR}/libzip_plugin.cpp
    ${PROJECT_SOURCE_DIR}/libzip_plugin.h
    ${BACKEND_DIR}/libzip_backend.cpp
    ${BACKEND_DIR}/libzip_backend.h
    ${BACKEND_DIR}/libzip_write.cpp
)

target_link_libraries(libzipplugin
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    compressor-interface
    ZLIB::ZLIB
    zip
)
