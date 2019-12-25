## deepin-compressor
Archive Manager is a software tool that provides common functions for Extracting files and compressing files.

归档管理器是一款快速、轻巧的解压缩软件，提供对文件解压缩的常用功能。

#### Core
The GUI is based on [DTK](https://github.com/linuxdeepin/dtkwidget), Qt (Supported >= 5.12).

Depends: libarchive13 (>= 3.2.1), libc6 (>= 2.14), libdisomaster, libdtkcore5 (>= 5.0.10+r1+g26d67ec), libdtkgui5 (>= 5.0.10), libdtkwidget5, libgcc1 (>= 1:3.0), libgl1, libglib2.0-0 (>= 2.16.0), libgsettings-qt1 (>= 0.1+14.04.20140408), libkf5codecs5 (>= 4.96.0), libpoppler-cpp0v5 (>= 0.46.0), libpulse-mainloop-glib0 (>= 0.99.1), libpulse0 (>= 0.99.1), libqt5concurrent5 (>= 5.0.2), libqt5core5a (>= 5.11.0~rc1), libqt5dbus5 (>= 5.0.2), libqt5gui5 (>= 5.7.0), libqt5multimedia5 (>= 5.6.0~beta), libqt5multimediawidgets5 (>= 5.6.0~beta), libqt5network5 (>= 5.0.2), libqt5svg5 (>= 5.6.0~beta), libqt5widgets5 (>= 5.11.0~rc1), libqt5x11extras5 (>= 5.6.0), libqt5xml5 (>= 5.0.2), libsecret-1-0 (>= 0.7), libstdc++6 (>= 5), libzip4 (>= 1.3.0)

Build-Depends: debhelper (>= 11), libgsettings-qt-dev, libsecret-1-dev, libpoppler-cpp-dev, libudisks2-qt5-dev, libdisomaster-dev, libkf5codecs-dev, libarchive-dev, libzip-dev, qttools5-dev-tools, deepin-gettext-tools, qtbase5-dev, qt5-default, libdtkwidget-dev,libqt5svg5-dev, libqt5x11extras5-dev, qtmultimedia5-dev
Standards-Version: 4.1.3

#### Installation
sudo apt-get install deepin-compressor

#### Build
- mkdir build
- cd build
- qmake ..
- make

Caution:
sudo cp plugin/*/*.so /usr/lib/deepin-compressor/plugins
sudo cp plugin/*/*/*.so /usr/lib/deepin-compressor/plugins