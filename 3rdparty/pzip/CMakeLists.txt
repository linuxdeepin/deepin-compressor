# pzip - 高性能并行 ZIP 压缩工具
cmake_minimum_required(VERSION 3.9.5)

set(PZIP_NAME pzip-tool)
project(${PZIP_NAME})

# pzip 不需要 Qt，禁用 AUTOMOC
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# 头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ZLIB_INCLUDE_DIRS})

# 源文件
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

# 编译成静态库
add_library(pzip_core_lib STATIC ${PZIP_SOURCES})
target_include_directories(pzip_core_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(pzip_core_lib ${ZLIB_LIBRARIES} Threads::Threads)

# 强制使用 C++17（覆盖父项目的 C++11 设置）
target_compile_features(pzip_core_lib PUBLIC cxx_std_17)

# 根据架构设置优化选项
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
    # ARM 架构优化
    target_compile_options(pzip_core_lib PRIVATE -O3 -ffast-math -funroll-loops)
else()
    # x86 架构优化
    target_compile_options(pzip_core_lib PRIVATE -O3 -march=native -ffast-math -funroll-loops)
endif()

# 编译 pzip 可执行文件
add_executable(pzip-bin cmd/pzip_main.cpp)
target_link_libraries(pzip-bin pzip_core_lib)
set_target_properties(pzip-bin PROPERTIES OUTPUT_NAME "pzip")

# 编译 punzip 可执行文件
add_executable(punzip-bin cmd/punzip_main.cpp)
target_link_libraries(punzip-bin pzip_core_lib)
set_target_properties(punzip-bin PROPERTIES OUTPUT_NAME "punzip")

# 安装规则 - 安装到 deepin-compressor 的 lib 目录
install(TARGETS pzip-bin punzip-bin
    RUNTIME DESTINATION lib/deepin-compressor
)

