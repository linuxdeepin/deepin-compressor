cmake_minimum_required(VERSION 3.9.5)

set(LIB_NAME compressor-interface)
project(archive-api)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

if (QT_DESIRED_VERSION MATCHES 6)
    find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Core5Compat)
endif()

include(FindPkgConfig)
pkg_search_module(MOUNT REQUIRED mount)

set(API_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${MOUNT_INCLUDE_DIRS})
include_directories(BEFORE ${API_DIR}/plugin)
include_directories(${API_DIR})
include_directories(${HFILES_OUTPUT_PATH})

set(API_SOURCES
    ${API_DIR}/archiveinterface.cpp
    ${API_DIR}/common.cpp
    ${API_DIR}/datamanager.cpp
    ${API_DIR}/queries.cpp
    ${API_DIR}/encoding/encoding_detector.cpp
    ${API_DIR}/plugin/kpluginfactory.cpp
    ${API_DIR}/plugin/kpluginloader.cpp
    ${API_DIR}/plugin/kpluginmetadata.cpp
)

# 旧 CLI 框架（cliinterface/cliproperties/archiveformat）已由 CliRunner 取代，
# 进程框架（kprocess/kpty）已由自有 clipzip 插件的 QProcess 取代，两者都不再编进契约层
set(PENDING_SOURCES
)

set(API_HEADERS
    ${API_DIR}/common.h
    ${API_DIR}/commonstruct.h
    ${API_DIR}/datamanager.h
    ${API_DIR}/queries.h
    ${API_DIR}/archiveinterface.h
    ${API_DIR}/plugin/kpluginfactory.h
    ${API_DIR}/plugin/kpluginloader.h
    ${API_DIR}/encoding/encoding_detector.h
    ${API_DIR}/plugin/kpluginmetadata.h
)

file(COPY ${API_HEADERS} DESTINATION ${HFILES_OUTPUT_PATH})

link_directories(${LIBRARY_OUTPUT_PATH})

add_library(${LIB_NAME} SHARED ${API_SOURCES} ${API_HEADERS} ${PENDING_SOURCES})

target_link_libraries(${LIB_NAME}
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
    KF${KF_VERSION_MAJOR}::Codecs
    ${MOUNT_LIBRARIES}
)

if (QT_DESIRED_VERSION MATCHES 6)
    target_link_libraries(${LIB_NAME} Qt${QT_DESIRED_VERSION}::Core5Compat)
endif()
