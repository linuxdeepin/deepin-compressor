# SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.9.5)

set(EXE_NAME archive)
project(${EXE_NAME}_test)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core Widgets Test)
include(FindPkgConfig)

pkg_search_module(MOUNT REQUIRED mount)

find_package(ZLIB REQUIRED)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
pkg_search_module(LIBZIP REQUIRED libzip)

include(CheckLibraryExists)
check_library_exists(util forkpty "" HAVE_UTIL_FORKPTY)

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/plugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/process)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libzipplugin)
include_directories(${LIBZIP_INCLUDE_DIRS})

# Archive layer include paths
set(ARCHIVE_SRC ${PROJECT_SOURCE_DIR}/../../../../src/archive)
include_directories(${ARCHIVE_SRC}/core)
include_directories(${ARCHIVE_SRC}/process)
include_directories(${ARCHIVE_SRC}/backends/legacy)

# Also need source/ for datamanager.h, etc.
include_directories(${PROJECT_SOURCE_DIR}/../../../../src/source)
include_directories(${PROJECT_SOURCE_DIR}/../../../../src/source/common)
include_directories(${PROJECT_SOURCE_DIR}/../../../../src/source/archivemanager)

link_directories(${LIBRARY_OUTPUT_PATH})

file(GLOB c_files ./*.cpp)

# Archive layer source files (tested directly, not via plugin loading).
# legacy_backend_factory_stub.cpp provides the stub for
# defaultLegacyInterfaceFactory() so legacy_backend_registry.cpp links
# without pulling in UiTools or KDE plugins.
set(ARCHIVE_SOURCES
    ${ARCHIVE_SRC}/core/archive_registry.cpp
    ${ARCHIVE_SRC}/core/registry_integration.cpp
    ${ARCHIVE_SRC}/core/known_differences.cpp
    ${ARCHIVE_SRC}/process/qt_process_runner.cpp
    ${ARCHIVE_SRC}/process/posix_pty_runner.cpp
    ${ARCHIVE_SRC}/backends/legacy/legacy_backend_adapter.cpp
    ${ARCHIVE_SRC}/backends/legacy/legacy_backend_observer_adapter.cpp
    ${ARCHIVE_SRC}/backends/legacy/legacy_backend_descriptors.cpp
    ${ARCHIVE_SRC}/backends/legacy/legacy_backend_registry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/legacy_backend_factory_stub.cpp
    ${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libzipplugin/libzipplugin.cpp
)

# Headers with Q_OBJECT that need moc output compiled into the test binary
set(ARCHIVE_HEADERS
    ${ARCHIVE_SRC}/process/process_runner.h
    ${ARCHIVE_SRC}/process/pty_runner.h
)

add_executable(${EXE_NAME}_test ${c_files} ${ARCHIVE_SOURCES} ${ARCHIVE_HEADERS})

add_test(NAME ${EXE_NAME}_test COMMAND ${EXE_NAME}_test)

# Ensure the test binary can find libcompressor-interface.so at runtime
# without requiring an external LD_LIBRARY_PATH.
set_target_properties(${EXE_NAME}_test PROPERTIES
    BUILD_RPATH "${CMAKE_BINARY_DIR}/3rdparty/interface"
    BUILD_RPATH_USE_ORIGIN TRUE)
set_tests_properties(${EXE_NAME}_test PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/3rdparty/interface"
    TIMEOUT 120)

target_link_libraries(${EXE_NAME}_test
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Test
    ${MOUNT_LIBRARIES}
    compressor-interface
    ZLIB::ZLIB
    KF${KF_VERSION_MAJOR}::Codecs
    ${LIBZIP_LIBRARIES}
    ${GTEST_LIBRARIES}

    pthread
)

if(HAVE_UTIL_FORKPTY)
    target_link_libraries(${EXE_NAME}_test util)
endif()
