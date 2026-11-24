# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME test)
project(${EXE_NAME})

add_subdirectory(UnitTest)
add_subdirectory(contract)

add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/archive_contract_test
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/test/run_isolated_gtest.py
            ${CMAKE_BINARY_DIR}/bin/tests/deepin-compressor_test
            --isolate-suite UT_MainWindow.
    COMMAND echo " =================== TEST END ==================== "
)

# debian 会自动执行 make test
#'make test'命令依赖与我们的测试程序
#add_dependencies(test ${PROJECT_NAME_TEST})

