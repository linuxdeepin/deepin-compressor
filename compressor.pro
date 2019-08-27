TEMPLATE = subdirs

SUBDIRS += \
    deepin-compressor \
    plugin/cli7zplugin \
    plugin/clirarplugin \
    plugin/cliunarchiverplugin \
    plugin/clizipplugin \
    plugin/libarchive \
    plugin/libsinglefileplugin \
    plugin/libzipplugin

deepin-compressor.depends = plugin/cli7zplugin
