#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(EXE_NAME deepin-compressor)
#set(APP_TRANS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/translate_generation.sh")
project(${EXE_NAME})

#配置版本号
configure_file(environments.h.in environments.h @ONLY)


#测试点
option(PERF_ON "open the info of performance testing" ON)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${HFILES_OUTPUT_PATH}/config.h)

#项目相关设置
#3.1 包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
#自动添加 CMAKE_CURRENT_BINARY_DIR 和 CMAKE_CURRENT_SOURCE_DIR 到当前处理的 CMakeLists.txt。
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()


#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Concurrent)
find_package(KF5Codecs REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
include(FindPkgConfig)


pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/source)
include_directories(${PROJECT_SOURCE_DIR}/source/archivemanager)
include_directories(${PROJECT_SOURCE_DIR}/source/common)
include_directories(${PROJECT_SOURCE_DIR}/source/dialog)
include_directories(${PROJECT_SOURCE_DIR}/source/dialog/openwithdialog)
include_directories(${PROJECT_SOURCE_DIR}/source/page)
include_directories(${PROJECT_SOURCE_DIR}/source/tree)
include_directories(${HFILES_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files RELATIVE ${PROJECT_SOURCE_DIR} source/*.cpp source/*.c main.cpp)
file(GLOB_RECURSE h_files RELATIVE ${PROJECT_SOURCE_DIR} source/*.h *.h)
file(GLOB_RECURSE res_files RELATIVE ${PROJECT_SOURCE_DIR} *.qrc)

#添加资源文件
QT5_ADD_RESOURCES(QRC_FILES ${res_files})

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#生成可执行程序
add_executable(${EXE_NAME}  ${c_files} ${h_files} ${QRC_FILES})

#链接动静态库
target_link_libraries(${EXE_NAME}
    Qt5::Widgets
    KF5::Codecs
    ${DtkWidget_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GIO_LIBRARIES}
    compressor-interface
)


set(CMAKE_INSTALL_PREFIX /usr)


install(TARGETS deepin-compressor
        DESTINATION bin/
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES  desktop/deepin-compressor.desktop
        DESTINATION share/applications/
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

file(GLOB INSTALL_QM_FILES ../translations/*.qm)
install(FILES ${INSTALL_QM_FILES} 
        DESTINATION share/deepin-compressor/translations)


# for i in `ls *.desktop`; do echo "install(FILES ${i} DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)";done

install(FILES desktop/deepin-compress.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-compress-multi.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES desktop/deepin-compressor.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES desktop/deepin-compress-to_7z.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES desktop/deepin-compress-to_zip.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompress.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompresshere.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompresshere-multi.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompresshere-split.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompresshere-split_multi.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES desktop/deepin-decompressmkdir.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES desktop/deepin-decompressmkdir-split.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompress-multi.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompress-split.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES desktop/deepin-decompress-split_multi.desktop DESTINATION share/deepin/dde-file-manager/oem-menuextensions/)

install(FILES assets/icons/deepin/builtin/icons/deepin-compressor.svg DESTINATION share/icons/hicolor/scalable/apps/)
install(FILES assets/mimetype/deepin-compressor.xml DESTINATION share/mime/packages/)

install(DIRECTORY assets/deepin-compressor DESTINATION /usr/share/deepin-manual/manual-assets/application/)
