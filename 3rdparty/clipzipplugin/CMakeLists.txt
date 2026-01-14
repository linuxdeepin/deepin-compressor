cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME clipzipplugin)
project(${LIB_NAME})

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)

include(FindPkgConfig)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})

link_directories(${LIBRARY_OUTPUT_PATH})

file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} *.h)
file(GLOB_RECURSE json_files RELATIVE ${PROJECT_SOURCE_DIR} *.json)

add_library(${LIB_NAME} SHARED ${c_files} ${json_files} ${h_files})

target_link_libraries(${LIB_NAME}
    Qt${QT_DESIRED_VERSION}::Widgets
    KF${KF_VERSION_MAJOR}::Codecs
    compressor-interface
)

