cmake_minimum_required(VERSION 3.9.5)

project(own-clirar)

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/clirar)
set(CLI_RUNNER_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/cli)
set(LEGACY_DIR ${CMAKE_SOURCE_DIR}/3rdparty/interface)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src/archive/api)
include_directories(${LEGACY_DIR}/archiveinterface)
include_directories(${BACKEND_DIR})
include_directories(${CLI_RUNNER_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_library(clirarplugin SHARED
    ${PROJECT_SOURCE_DIR}/clirar_plugin.cpp
    ${PROJECT_SOURCE_DIR}/clirar_plugin.h
    ${BACKEND_DIR}/clirar_backend.cpp
    ${BACKEND_DIR}/clirar_backend.h
    ${CLI_RUNNER_DIR}/cli_runner.cpp
    ${CLI_RUNNER_DIR}/cli_runner.h
)

target_link_libraries(clirarplugin
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    compressor-interface
    util
)
