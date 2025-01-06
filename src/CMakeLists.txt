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
include(FindPkgConfig)

set(qt_required_components Core DBus Gui Widgets Concurrent)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND qt_required_components Core5Compat)
endif()

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS ${qt_required_components})
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(KF${KF_VERSION_MAJOR}Codecs REQUIRED)

set(LINK_LIBS
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::DBus
    Qt${QT_DESIRED_VERSION}::Gui
    Qt${QT_DESIRED_VERSION}::Widgets
    Qt${QT_DESIRED_VERSION}::Concurrent
    Dtk${DTK_VERSION_MAJOR}::Widget
    KF${KF_VERSION_MAJOR}::Codecs
)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND LINK_LIBS Qt${QT_DESIRED_VERSION}::Core5Compat)
endif()


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

# 根据Qt版本选择合适的命令
if (QT_VERSION VERSION_GREATER_EQUAL "6.0")
    qt_add_resources(QRC_FILES ${res_files})
else()
    qt5_add_resources(QRC_FILES ${res_files})
endif()


#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#生成可执行程序
add_executable(${EXE_NAME}  ${c_files} ${h_files} ${QRC_FILES})

#链接动静态库
target_link_libraries(${EXE_NAME}
    ${LINK_LIBS}
    ${GOBJECT_LIBRARIES}
    ${GIO_LIBRARIES}
    compressor-interface
)

#hw机型增加DConfig配置
set(APPID org.deepin.compressor)
set(configFile ${PROJECT_SOURCE_DIR}/assets/org.deepin.compressor.method.json)
if (DEFINED DSG_DATA_DIR)
    message("-- DConfig is supported by DTK")
    if (DTK_VERSION_MAJOR EQUAL "6")
        dtk_add_config_meta_files(APPID ${APPID} FILES ${configFile})
    else()
        dconfig_meta_files(APPID ${APPID} FILES ${configFile})
    endif()
endif()

install(TARGETS deepin-compressor DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES desktop/deepin-compressor.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

# translation files
TRANSLATION_GENERATE(QM_FILES ${CMAKE_SOURCE_DIR}/translations)
add_custom_target(${PROJECT_NAME}_qm_files DEPENDS ${QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_qm_files)

# Install translations
install(FILES ${QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)


# 自定义右键菜单
install(FILES conf/compressor-singlecompress.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/context-menus/)
install(FILES conf/compressor-multicompress.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/context-menus/)
install(FILES conf/compressor-singleextract.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/context-menus/)
install(FILES conf/compressor-multiextract.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/context-menus/)

install(FILES assets/icons/deepin/builtin/icons/deepin-compressor.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/)
install(FILES assets/mimetype/deepin-compressor.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages/)

# 集成帮助手册
install(DIRECTORY assets/deepin-compressor DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)

# 支持压缩的格式
#install(FILES conf/compressor-singlecompress.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-compressor/compressedformat/)
