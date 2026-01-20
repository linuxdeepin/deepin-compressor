add_subdirectory(ChardetDetector)

add_subdirectory(interface)

add_subdirectory(cli7zplugin)

add_subdirectory(clirarplugin)

add_subdirectory(libarchive)

add_subdirectory(libpigzplugin)

add_subdirectory(libsinglefileplugin)

add_subdirectory(libzipplugin)

add_subdirectory(libminizipplugin)

# clipzipplugin（仅 x86/ARM + Qt6 环境启用，依赖 src/pzip）
if((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm|x86_64|amd64") AND (QT_VERSION_MAJOR EQUAL 6))
    message(STATUS "x86/ARM + Qt6 环境，启用 pzip 高性能压缩插件")
    add_subdirectory(clipzipplugin)
else()
    message(STATUS "非 x86/ARM + Qt6 环境，使用默认 libzip 插件")
endif()

##add_subdirectory(cliunarchiverplugin) #不可用

##add_subdirectory(clizipplugin) #不可用







