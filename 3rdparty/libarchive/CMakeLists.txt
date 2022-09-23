# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#定义cmake版本
project(libarchive)

add_subdirectory(libarchive)

add_subdirectory(readwritelibarchiveplugin)

add_subdirectory(readonlylibarchiveplugin)






