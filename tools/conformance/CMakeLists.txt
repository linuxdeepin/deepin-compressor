# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# 归档后端一致性观测器（conformance）
# 不编入默认构建：cmake -DBUILD_CONFORMANCE=ON

cmake_minimum_required(VERSION 3.9.5)

set(EXE_NAME conformance)
project(${EXE_NAME})

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)
include(FindPkgConfig)
pkg_search_module(MOUNT REQUIRED mount)

if (QT_DESIRED_VERSION MATCHES 6)
    find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()

include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${MOUNT_INCLUDE_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/tests)
link_directories(${LIBRARY_OUTPUT_PATH})

file(GLOB c_files ./*.cpp)
file(GLOB h_files ./*.h)

# 复用生产的 MIME 判定逻辑，保证观测条件与真实业务一致
set(PROD_SOURCES ${CMAKE_SOURCE_DIR}/src/source/common/mimetypes.cpp)

add_executable(${EXE_NAME} ${c_files} ${h_files} ${PROD_SOURCES})
target_compile_definitions(${EXE_NAME} PRIVATE REPO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/source ${CMAKE_SOURCE_DIR}/src/source/common)

target_link_libraries(${EXE_NAME}
    Qt${QT_DESIRED_VERSION}::Widgets
    KF${KF_VERSION_MAJOR}::Codecs
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Widget
    ${MOUNT_LIBRARIES}
    compressor-interface
)

if (QT_DESIRED_VERSION MATCHES 6)
    target_link_libraries(${EXE_NAME} Qt${QT_DESIRED_VERSION}::Core5Compat)
endif()

# 让构建产物可直接链接构建树里的 compressor-interface
set_target_properties(${EXE_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH FALSE
    BUILD_RPATH "${LIBRARY_OUTPUT_PATH};${CMAKE_BINARY_DIR}/3rdparty/interface"
)
