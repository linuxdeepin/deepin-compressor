#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME test)
project(${EXE_NAME})

add_subdirectory(UnitTest)

add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
#    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test-prj-running.sh
    COMMAND ${CMAKE_BINARY_DIR}/bin/bz2plugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/cli7zplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/clirarplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/deepin-compressor_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/gzplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/interface_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/libarchive_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/libminizipplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/libzipplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/readonlylibarchiveplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/readwritelibarchiveplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/singlefile_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/xzplugin_test
    COMMAND echo " =================== TEST END ==================== "
)

# debian 会自动执行 make test
#'make test'命令依赖与我们的测试程序
#add_dependencies(test ${PROJECT_NAME_TEST})

