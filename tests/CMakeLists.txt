# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME test)
project(${EXE_NAME})

add_subdirectory(UnitTest)

# enable_testing() is called in the top-level CMakeLists.txt, which
# provides the standard CMake 'test' target (runs ctest).  We create an
# ALL build target so 'make' (or 'make all') builds all test executables,
# and 'make test' (ctest) finds them ready to run.  This preserves the
# Debian and existing development workflow where 'make test' works
# without a separate build step.

add_custom_target(build_tests ALL
    DEPENDS
        bz2plugin_test
        cli7zplugin_test
        clirarplugin_test
        deepin-compressor_test
        gzplugin_test
        interface_test
        libminizipplugin_test
        libpigzplugin_test
        libzipplugin_test
        readonlylibarchiveplugin_test
        readwritelibarchiveplugin_test
        xzplugin_test
        contract_test
        archive_test
)

# debian 会自动执行 make test
#'make test'命令依赖与我们的测试程序
