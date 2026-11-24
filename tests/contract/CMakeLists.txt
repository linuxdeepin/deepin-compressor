# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.9.5)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/tests)
set(EXE_NAME archive_contract_test)
project(${EXE_NAME})

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core Widgets)
if (QT_DESIRED_VERSION MATCHES 6)
    find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(KF${KF_VERSION_MAJOR}Archive REQUIRED)
find_package(GTest REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/archive/core
    ${CMAKE_SOURCE_DIR}/src/archive/encoding
    ${CMAKE_SOURCE_DIR}/src/source/archivemanager
    ${CMAKE_SOURCE_DIR}/src/source/common
    ${CMAKE_SOURCE_DIR}/src/source/dialog
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive
    ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/pigz
    ${CMAKE_SOURCE_DIR}/src/archive/backends/sevenzip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/clipzip
    ${CMAKE_SOURCE_DIR}/src/archive/backends/rar
    ${HFILES_OUTPUT_PATH}
)

add_executable(${EXE_NAME}
    main.cpp
    api_smoke_test.cpp
    long_name_test.cpp
    singlefile_contract_test.cpp
    libzip_contract_test.cpp
    libarchive_contract_test.cpp
    minizip_contract_test.cpp
    pigz_contract_test.cpp
    cli7z_contract_test.cpp
    clirar_contract_test.cpp
    clipzip_contract_test.cpp
    registry_test.cpp
    bridge_test.cpp
    encoding_test.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/archive_registry.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/path_guard.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/extraction_output.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/staging_merge.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/encoding_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/encoding/encoding_detector.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/core/archive_registry.h
    ${CMAKE_SOURCE_DIR}/src/source/archivemanager/archive_session.cpp
    ${CMAKE_SOURCE_DIR}/src/source/archivemanager/archive_session.h
    ${CMAKE_SOURCE_DIR}/src/source/dialog/archivequeries.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/singlefile_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/singlefile_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/stream_decoder.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/singlefile/stream_decoder.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip/libzip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libzip/libzip_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive/libarchive_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/libarchive/libarchive_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip/minizip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/minizip/minizip_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/pigz/pigz_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/pigz/pigz_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/sevenzip/sevenzip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/sevenzip/sevenzip_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/rar/rar_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/rar/rar_backend.h
    ${CMAKE_SOURCE_DIR}/src/archive/backends/clipzip/clipzip_backend.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/backends/clipzip/clipzip_backend.h
)

target_compile_definitions(${EXE_NAME} PRIVATE
    CONTRACT_TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}/tests/UnitTest"
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
    z
    bz2
    lzma
    zip
    archive
    minizip
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
