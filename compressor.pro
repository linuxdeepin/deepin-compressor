TEMPLATE = subdirs

SUBDIRS += \
    deepin-compressor \
    plugin

deepin-compressor.depends = plugin
