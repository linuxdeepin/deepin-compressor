# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.9.5)

set(EXE_NAME contract)
project(${EXE_NAME}_test)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(KF${KF_VERSION_MAJOR}Archive REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

if (QT_DESIRED_VERSION MATCHES 6)
    find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()

find_package(ZLIB REQUIRED)
find_package(LibArchive REQUIRED)
include(FindPkgConfig)

pkg_search_module(MOUNT REQUIRED mount)

# Header search paths for all plugin and interface headers
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/archiveinterface)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/plugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/interface/process)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/cli7zplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/clirarplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libzipplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libminizipplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libpigzplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libarchive/libarchive)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libarchive/readonlylibarchiveplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libarchive/readwritelibarchiveplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libsinglefileplugin/singlefile)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libsinglefileplugin/gzplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libsinglefileplugin/bz2plugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libsinglefileplugin/xzplugin)
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/ChardetDetector)

link_directories(${LIBRARY_OUTPUT_PATH})

# Ensure linker can find libarchive (transitive dependency from compressor-archive
# via plugin shared libraries).
if(LibArchive_LIBRARIES)
    get_filename_component(LIBARCHIVE_LIB_DIR "${LibArchive_LIBRARIES}" DIRECTORY)
    link_directories(${LIBARCHIVE_LIB_DIR})
endif()

# Only compile the contract test sources — plugin sources come from the
# pre-built shared libraries to avoid Q_PLUGIN_METADATA symbol conflicts.
file(GLOB c_files ./*.cpp)
file(GLOB h_files ./*.h)

add_executable(${EXE_NAME}_test ${c_files} ${h_files})

target_link_libraries(${EXE_NAME}_test
    Qt${QT_DESIRED_VERSION}::Widgets
    KF${KF_VERSION_MAJOR}::Codecs
    KF${KF_VERSION_MAJOR}::Archive
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${MOUNT_LIBRARIES}
    # Pre-built plugin shared libraries (each carries its own Q_PLUGIN_METADATA)
    cli7zplugin
    clirarplugin
    libzipplugin
    minizipplugin
    libpigzplugin
    readonlylibarchiveplugin
    readwritelibarchiveplugin
    gzplugin
    bz2plugin
    xzplugin
    # libarchive transitive dependencies
    acl
    xml2
    # Base libraries
    compressor-interface
    ZLIB::ZLIB
    compressor-singlefile
    compressor-ChardetDetector
    ${GTEST_LIBRARIES}
    pthread
)

if (QT_DESIRED_VERSION MATCHES 6)
    target_link_libraries(${EXE_NAME}_test
        Qt${QT_DESIRED_VERSION}::Core5Compat
    )
endif()
