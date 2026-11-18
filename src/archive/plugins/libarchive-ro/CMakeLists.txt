cmake_minimum_required(VERSION 3.9.5)

project(own-libarchive-ro)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

set(BACKEND_DIR ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src/archive/api)
include_directories(${BACKEND_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${LIBARCHIVE_INCLUDE_DIRS})

add_library(readonlylibarchiveplugin SHARED
    ${PROJECT_SOURCE_DIR}/libarchive_ro_plugin.cpp
    ${PROJECT_SOURCE_DIR}/libarchive_ro_plugin.h
    ${BACKEND_DIR}/archive_backend.cpp
    ${BACKEND_DIR}/archive_backend.h
    ${BACKEND_DIR}/archive_write.cpp
)

target_link_libraries(readonlylibarchiveplugin
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    compressor-interface
    ${LIBARCHIVE_LIBRARIES}
)

link_directories(${LIBARCHIVE_LIBRARY_DIRS})
