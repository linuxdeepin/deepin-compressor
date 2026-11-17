# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME readonlylibarchiveplugin)
project(${EXE_NAME}_test)

# Access private members in existing UT code
add_compile_options(-fno-access-control)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(KF${KF_VERSION_MAJOR}Archive REQUIRED)
find_package(LibArchive)
find_package(ZLIB REQUIRED)
include(FindPkgConfig)


#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/../${3RDPARTYRELATIVEPATH}/libarchive/readonlylibarchiveplugin)
include_directories(${PROJECT_SOURCE_DIR}/../${3RDPARTYRELATIVEPATH}/libarchive/libarchive)

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ../${3RDPARTYRELATIVEPATH}/libarchive/readonlylibarchiveplugin/*.cpp
    ../${3RDPARTYRELATIVEPATH}/libarchive/libarchive/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ../${3RDPARTYRELATIVEPATH}/libarchive/readonlylibarchiveplugin/*.h
    ../${3RDPARTYRELATIVEPATH}/libarchive/libarchive/*.h
    ./*.h
)

file(GLOB_RECURSE json_files
    ../${3RDPARTYRELATIVEPATH}/libarchive/readonlylibarchiveplugin/*.json
    ./*.h
)

#生成可执行文件
find_library(LIBZSTD zstd)
find_library(LIBLZMA lzma)
find_library(LIBLZ4 lz4)
find_library(LIBBZ2 bz2)
find_library(LIBNETTLE nettle HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
find_library(LIBHOGWEED hogweed HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
find_library(LIBACL acl)
find_library(LIBB2 b2 HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
find_library(LIBBSD bsd HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
find_library(LIBCRYPTO crypto)
find_library(LIBXML2 xml2)

add_executable(${EXE_NAME}_test ${h_files} ${c_files} ${json_files})

add_test(NAME ${EXE_NAME}_test COMMAND ${EXE_NAME}_test)

#链接动静态库
# Collect only successfully-found transitive dependencies to avoid
# linking against *-NOTFOUND which breaks the build in clean environments.
set(LIBARCHIVE_TRANSITIVE_DEPS "")
foreach(_dep LIBZSTD LIBLZMA LIBLZ4 LIBBZ2 LIBNETTLE LIBHOGWEED LIBACL LIBB2 LIBBSD LIBCRYPTO LIBXML2)
    if(${_dep})
        list(APPEND LIBARCHIVE_TRANSITIVE_DEPS ${${_dep}})
    endif()
endforeach()

target_link_libraries(${EXE_NAME}_test
    Qt${QT_DESIRED_VERSION}::Widgets
    KF${KF_VERSION_MAJOR}::Codecs
    KF${KF_VERSION_MAJOR}::Archive
    ${LibArchive_LIBRARIES}
    ZLIB::ZLIB
    ${LIBARCHIVE_TRANSITIVE_DEPS}
    compressor-interface
#    compressor-archive
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
