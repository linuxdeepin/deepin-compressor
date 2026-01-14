# pzip - 高性能并行 ZIP 压缩工具
cmake_minimum_required(VERSION 3.9.5)

set(PZIP_NAME pzip-tool)
project(${PZIP_NAME})

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ZLIB_INCLUDE_DIRS})

set(PZIP_SOURCES
    src/archiver.cpp
    src/extractor.cpp
    src/file_task.cpp
    src/zip_writer.cpp
    src/zip_reader.cpp
    src/fast_deflate.cpp
    src/utils.cpp
    src/worker_pool.cpp
)

add_library(pzip_core_lib STATIC ${PZIP_SOURCES})
target_include_directories(pzip_core_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(pzip_core_lib ${ZLIB_LIBRARIES} Threads::Threads)

target_compile_features(pzip_core_lib PUBLIC cxx_std_17)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
    # ARM 架构优化
    target_compile_options(pzip_core_lib PRIVATE -O3 -ffast-math -funroll-loops)
else()
    # x86 架构优化
    target_compile_options(pzip_core_lib PRIVATE -O3 -march=native -ffast-math -funroll-loops)
endif()

add_executable(pzip-bin cmd/pzip_main.cpp)
target_link_libraries(pzip-bin pzip_core_lib)
set_target_properties(pzip-bin PROPERTIES OUTPUT_NAME "pzip")

add_executable(punzip-bin cmd/punzip_main.cpp)
target_link_libraries(punzip-bin pzip_core_lib)
set_target_properties(punzip-bin PROPERTIES OUTPUT_NAME "punzip")

install(TARGETS pzip-bin punzip-bin
    RUNTIME DESTINATION lib/deepin-compressor
)

