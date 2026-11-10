# pzip - 高性能并行 ZIP 压缩工具
# 安全说明：仅构建 pzip（压缩），不构建 punzip（解压）。
# punzip 的解压路径存在符号链接竞态（symlink race）安全漏洞（PMS BUG-382553），
# 已从 master 分支构建目标中移除，本移植保持一致——pzip 为 write-only 工具。
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

find_package(OpenSSL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ZLIB_INCLUDE_DIRS})

set(PZIP_SOURCES
    src/archiver.cpp
    src/file_task.cpp
    src/zip_writer.cpp
    src/zip_reader.cpp
    src/fast_deflate.cpp
    src/utils.cpp
    src/worker_pool.cpp
    src/crypto/aes_encryptor.cpp
    src/crypto/winzip_aes_extra.cpp
)

add_library(pzip_core_lib STATIC ${PZIP_SOURCES})
target_include_directories(pzip_core_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(pzip_core_lib
    ${ZLIB_LIBRARIES}
    Threads::Threads
    OpenSSL::Crypto
)

target_compile_features(pzip_core_lib PUBLIC cxx_std_17)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
    target_compile_options(pzip_core_lib PRIVATE -O3 -funroll-loops -march=armv8-a+crc)
else()
    target_compile_options(pzip_core_lib PRIVATE -O3 -mtune=generic -funroll-loops)
endif()

add_executable(pzip-bin cmd/pzip_main.cpp)
target_link_libraries(pzip-bin pzip_core_lib)
set_target_properties(pzip-bin PROPERTIES OUTPUT_NAME "pzip")

install(TARGETS pzip-bin
    RUNTIME DESTINATION lib/deepin-compressor
)

