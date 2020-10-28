#!/bin/bash
# 该脚本用于修改代码编译后使用，同 make install 操作

# 更新库
sudo rm -rf /usr/lib/deepin-compressor/plugins/*
sudo cp output/compressor-lib/*.so /usr/lib/deepin-compressor/plugins/

# 更新可执行程序
sudo cp output/deepin-compressor /usr/bin/

# 更新.desktop
sudo cp src/desktop/*.desktop /usr/share/deepin/dde-file-manager/oem-menuextensions/
sudo cp src/desktop/*or.desktop /usr/share/applications/
# 去除多余.desktop
sudo rm /usr/share/deepin/dde-file-manager/oem-menuextensions/*or.desktop
sudo rm /usr/share/deepin/dde-file-manager/oem-menuextensions/deepin-compress-to*.desktop
sudo rm /usr/share/deepin/dde-file-manager/oem-menuextensions/deepin-decompressmkdi*.desktop

# 更新翻译文件
# lupdate deepin-compressor/ -ts translations/*.ts -no-obsolete
# lrelease translations/*.ts
sudo cp translations/*.qm /usr/share/deepin-compressor/translations

# #忽略文件改动
# git update-index --assume-unchanged translations/*.ts
# git update-index --assume-unchanged translations/*.qm
# #不忽略文件改动
# git update-index --no-assume-unchanged translations/*.ts
# git update-index --no-assume-unchanged translations/*.qm

read -s -t 2 -p "Please input Enter! " 