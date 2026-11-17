cmake_minimum_required(VERSION 3.9.5)

project(own-singlefile)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)
find_package(ZLIB REQUIRED)
find_library(BZ2_LIBRARY bz2 REQUIRED)
find_library(LZMA_LIBRARY lzma REQUIRED)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src/archive/api)
include_directories(${BACKEND_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_library(own-singlefile STATIC
    ${BACKEND_DIR}/stream.cpp
    ${BACKEND_DIR}/single_file_backend.cpp
)

target_link_libraries(own-singlefile PUBLIC
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    compressor-interface
    ZLIB::ZLIB
    ${BZ2_LIBRARY}
    ${LZMA_LIBRARY}
)

foreach(fmt gz bz2 xz)
    add_library(${fmt}plugin SHARED
        ${PROJECT_SOURCE_DIR}/${fmt}_plugin.cpp
        ${PROJECT_SOURCE_DIR}/${fmt}_plugin.h
    )
    target_link_libraries(${fmt}plugin own-singlefile)
endforeach()
