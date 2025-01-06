project(clizipplugin)

cmake_minimum_required(VERSION 3.9.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(LIB_NAME clizipplugin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS Widgets)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Widget)

include(FindPkgConfig)



include_directories(${PROJECT_SOURCE_DIR}/../ChardetDetector)
include_directories(${PROJECT_SOURCE_DIR}/../interface)

file(GLOB_RECURSE json_files  *.json)



file(GLOB_RECURSE c_files  *.cpp)
file(GLOB_RECURSE h_files  *.h)
file(GLOB_RECURSE json_files  *.json)
include_directories(${PROJECT_SOURCE_DIR})

add_library(${LIB_NAME} SHARED  ${c_files} ${h_files} ${json_files})
target_link_libraries(${LIB_NAME}
    Qt${QT_DESIRED_VERSION}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    )

target_link_libraries( ${LIB_NAME} compressor-interface)
