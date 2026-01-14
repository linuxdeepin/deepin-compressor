add_subdirectory(ChardetDetector)

add_subdirectory(interface)

add_subdirectory(cli7zplugin)

add_subdirectory(clirarplugin)

add_subdirectory(libarchive)

add_subdirectory(libpigzplugin)

add_subdirectory(libsinglefileplugin)

add_subdirectory(libzipplugin)

add_subdirectory(libminizipplugin)

# pzip 高性能并行压缩工具（仅 ARM 环境启用）
# 检查是否为 ARM 架构
if((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm"))
    message(STATUS "ARM环境，启用 pzip 高性能压缩插件")
    add_subdirectory(pzip)
    add_subdirectory(clipzipplugin)
else()
    message(STATUS "非 ARM 环境，使用默认 libzip 插件")
endif()

##add_subdirectory(cliunarchiverplugin) #不可用

##add_subdirectory(clizipplugin) #不可用







