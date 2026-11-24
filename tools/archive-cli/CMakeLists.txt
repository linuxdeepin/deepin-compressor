# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.9.5)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/tools)
set(EXE_NAME archive-cli)
project(${EXE_NAME})

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core Widgets)
if (QT_DESIRED_VERSION MATCHES 6)
    find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(KF${KF_VERSION_MAJOR}Archive REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/archive/core
    ${CMAKE_SOURCE_DIR}/src/archive/encoding
    ${CMAKE_SOURCE_DIR}/src/archive/adapters
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive
    ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/pigz
    ${CMAKE_SOURCE_DIR}/src/archive/backends/sevenzip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/rar
    ${CMAKE_SOURCE_DIR}/src/archive/backends/clipzip
    ${CMAKE_SOURCE_DIR}/3rdparty/interface
    ${CMAKE_SOURCE_DIR}/3rdparty/interface/archiveinterface
    ${CMAKE_SOURCE_DIR}/3rdparty/interface/plugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libsinglefileplugin/singlefile
    ${CMAKE_SOURCE_DIR}/3rdparty/libsinglefileplugin/gzplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libsinglefileplugin/bz2plugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libsinglefileplugin/xzplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libzipplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libminizipplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libpigzplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/libarchive
    ${CMAKE_SOURCE_DIR}/3rdparty/libarchive/readwritelibarchiveplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/cli7zplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/clirarplugin
    ${CMAKE_SOURCE_DIR}/3rdparty/clipzipplugin
    ${HFILES_OUTPUT_PATH}
)

add_executable(${EXE_NAME}
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/archive_registry.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/encoding_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/path_guard.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/extraction_output.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/staging_merge.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_singlefile_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_libzip_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_libarchive_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_minizip_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_pigz_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_cli7z_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_clirar_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/adapters/legacy_clipzip_adapter.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/singlefile_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/stream_decoder.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip/libzip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive/libarchive_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip/minizip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/pigz/pigz_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/sevenzip/sevenzip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/rar/rar_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/clipzip/clipzip_backend.cpp
)

target_compile_definitions(${EXE_NAME} PRIVATE
    ARCHIVE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
if(TARGET pzip-bin)
    add_dependencies(${EXE_NAME} pzip-bin)
    target_compile_definitions(${EXE_NAME} PRIVATE DC_BUILT_PZIP="$<TARGET_FILE:pzip-bin>")
endif()

target_link_libraries(${EXE_NAME}
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Core5Compat
    Dtk${DTK_VERSION_MAJOR}::Widget
    KF${KF_VERSION_MAJOR}::Codecs
    KF${KF_VERSION_MAJOR}::Archive
    compressor-interface
    compressor-singlefile
    gzplugin
    bz2plugin
    xzplugin
    libzipplugin
    readwritelibarchiveplugin
    minizipplugin
    libpigzplugin
    cli7zplugin
    clirarplugin
    clipzipplugin
    z
    bz2
    lzma
    zip
    minizip
    archive
)
