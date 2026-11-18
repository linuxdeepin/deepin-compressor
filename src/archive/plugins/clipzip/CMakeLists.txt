cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME clipzipplugin)
project(own-clipzip)

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets Core5Compat)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})

add_library(${LIB_NAME} SHARED
    ${PROJECT_SOURCE_DIR}/clipzipplugin.cpp
    ${PROJECT_SOURCE_DIR}/clipzipplugin.h
    ${PROJECT_SOURCE_DIR}/kerfuffle_clipzip.json
)

target_link_libraries(${LIB_NAME}
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Core5Compat
    KF${KF_VERSION_MAJOR}::Codecs
    compressor-interface
)
