## deepin-compressor
Archive Manager is a fast and lightweight application for creating and extracting archives.

归档管理器是一款提供对文件解压、压缩常用功能的软件工具。

#### Core
The GUI is based on [DTK](https://github.com/linuxdeepin/dtkwidget), Qt (Supported >= 5.12).

Build-Depends:
debhelper (>= 11), cmake, libgsettings-qt-dev, libarchive-dev, libsecret-1-dev, libpoppler-cpp-dev, libudisks2-qt5-dev, libdisomaster-dev, libkf5codecs-dev, libzip-dev, qttools5-dev-tools, deepin-gettext-tools, qtbase5-dev, qtchooser (>= 55-gc9562a1-1~), qt5-qmake, libdtkwidget-dev,libqt5svg5-dev, libqt5x11extras5-dev, libkf5archive-dev, libminizip-dev, qttools5-dev, libgtest-dev

Execute Depends:
${shlibs:Depends}, ${misc:Depends}, p7zip-full, deepin-shortcut-viewer, unar[i386 amd64], rar[i386 amd64], unrar[i386 amd64]

### Third Party Code
[KDE ark](https://github.com/kde/ark)

#### Installation
sudo apt-get install deepin-compressor

#### Build
- mkdir build
- cd build
- cmake ..
- make
- make install

## Documentations

- [Development Documentation](https://linuxdeepin.github.io/)
 - [User Documentation](https://wikidev.uniontech.com/index.php?title=%E9%9F%B3%E4%B9%90) | [用户文档](https://wikidev.uniontech.com/index.php?title=音乐)

## Getting help

- [Official Forum](https://bbs.deepin.org/) for generic discussion and help.
- [Developer Center](https://github.com/linuxdeepin/developer-center) for BUG report and suggestions.
- [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
- [Translate for your language on Transifex](#) *please update to the actual Transifex link of this project*

## License

License description here. The license name is suggested to use the same one as [SPDX license identifier](https://spdx.org/licenses). Following is an example:

Deepin Example is licensed under [GPL-3.0-or-later](LICENSE)

## [Wiki](https://github.com/linuxdeepin/developer-center/wiki/%E9%97%AE%E9%A2%98%E5%8F%8D%E9%A6%88%E6%8C%87%E5%8D%97)

