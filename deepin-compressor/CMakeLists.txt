#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME deepin-compressor)
project(${EXE_NAME})

#配置版本号
configure_file(environments.h.in environments.h @ONLY)

#SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


#3.项目相关设置
#3.1 包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#3.3 打开全局moc
set(CMAKE_AUTOMOC ON)
#set(CMAKE_CXX_FLAGS "-std=c++11")
#set(CMAKE_CXX_FLAGS_DEBUG "-O0")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


#demo start
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ldtkwidget -ldtkgui -ldtkcore")#${EXE_NAME}
#-ldtkwidget -ldtkgui -ldtkcore
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()


#4 查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5 REQUIRED Widgets)
find_package(Qt5Concurrent)
find_package(Qt5Svg REQUIRED SvgWidgets)
find_package(KF5Codecs REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
include(FindPkgConfig)


pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

pkg_search_module(DtkCore REQUIRED dtkcore)
include_directories(${DtkCore_INCLUDE_DIRS})

pkg_search_module(GOBJECT REQUIRED gobject-2.0)
include_directories(${GOBJECT_INCLUDE_DIRS})

pkg_search_module(GIO REQUIRED gio-unix-2.0)
include_directories(${GIO_INCLUDE_DIRS})

pkg_search_module(DtkGui REQUIRED dtkgui)	
include_directories(${DtkGui_INCLUDE_DIRS})

set(LIBS_INCLUDE_DIRS 
	${DtkWidget_LIBRARIES}
	${DtkCore_LIBRARIES}
	${GOBJECT_LIBRARIES}
	${GIO_LIBRARIES}
	${DtkGui_LIBRARIES}
)



 
#6. 应用interface项目
#interface的h文件
include_directories(${PROJECT_SOURCE_DIR}/../3rdparty/interface)
#add_subdirectory(interface)

#7.主程序的h文件
include_directories(${PROJECT_SOURCE_DIR}/source/inc)
include_directories(${PROJECT_SOURCE_DIR}/source/inc/openwithdialog)

#8.主程序的源文件和头文件
#file(GLOB_RECURSE c_files  source/*.cpp source/*.c main.cpp)

set(c_files 
	source/src/archivemodel.cpp
	source/src/archivesortfiltermodel.cpp
        source/src/batchcompress.cpp
	source/src/batchextract.cpp
	source/src/compressor_fail.cpp
	source/src/compressor_success.cpp
    source/src/compressorapplication.cpp
	source/src/compresspage.cpp
	source/src/compresssetting.cpp
	source/src/customwidget.cpp
	source/src/ddesktopservicesthread.cpp
	source/src/encodingpage.cpp
	source/src/encryptionpage.cpp
	source/src/extractpausedialog.cpp
	source/src/fileViewer.cpp
	source/src/homepage.cpp
	source/src/lib_edit_button.cpp
	source/src/logviewheaderview.cpp
	source/src/logwidget.cpp
	source/src/mainwindow.cpp
	source/src/mimetypedisplaymanager.cpp
	source/src/monitorAdaptor.cpp
	source/src/monitorInterface.cpp
	source/src/myfileitem.cpp
	source/src/myfilesystemmodel.cpp
	source/src/openloadingpage.cpp
	source/src/progress.cpp
	source/src/progressdialog.cpp
	source/src/settingdialog.cpp
	source/src/settings_translation.cpp
	source/src/uncompresspage.cpp
	source/src/utils.cpp
	source/src/openwithdialog/basedialog.cpp
	source/src/openwithdialog/desktopfile.cpp
	source/src/openwithdialog/dfmstandardpaths.cpp
	source/src/openwithdialog/dmimedatabase.cpp
	source/src/openwithdialog/durl.cpp
	source/src/openwithdialog/mimesappsmanager.cpp
	source/src/openwithdialog/openwithdialog.cpp
	source/src/openwithdialog/properties.cpp
	main.cpp
)


set(h_files 
	source/inc/archivemodel.h
	source/inc/archivesortfiltermodel.h
	source/inc/batchcompress.h
	source/inc/batchextract.h
	source/inc/com.archive.mainwindow.monitor.xml
	source/inc/compressorapplication.h
	source/inc/compressor_fail.h
	source/inc/compressor_success.h
	source/inc/compresspage.h
	source/inc/compresssetting.h
	source/inc/customwidget.h
	source/inc/ddesktopservicesthread.h
	source/inc/encodingpage.h
	source/inc/encryptionpage.h
	source/inc/extractpausedialog.h
	source/inc/fileViewer.h
	source/inc/homepage.h
	source/inc/lib_edit_button.h
	source/inc/logviewheaderview.h
	source/inc/logwidget.h
	source/inc/mainwindow.h
	source/inc/mimetypedisplaymanager.h
	source/inc/monitorAdaptor.h
	source/inc/monitorInterface.h
	source/inc/myfileitem.h
	source/inc/myfilesystemmodel.h
	source/inc/openloadingpage.h
	source/inc/progressdialog.h
	source/inc/progress.h
	source/inc/settingdialog.h
	source/inc/uncompresspage.h
	source/inc/utils.h
	source/inc/openwithdialog/basedialog.h
	source/inc/openwithdialog/desktopfile.h
	source/inc/openwithdialog/dfmstandardpaths.h
	source/inc/openwithdialog/dmimedatabase.h
	source/inc/openwithdialog/durl.h
	source/inc/openwithdialog/mimesappsmanager.h
	source/inc/openwithdialog/openwithdialog.h
	source/inc/openwithdialog/properties.h
	source/inc/openwithdialog/singleton.h

)



file(GLOB_RECURSE res_files *.qrc)
set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../translations")
file( GLOB TS_FILES    ${TS_DIR}/*.ts)

QT5_ADD_RESOURCES(QRC_MOC ${res_files})

find_program(LUPDATE_EXECUTABLE lupdate)

foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
    message("${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file}")
endforeach()
#lupdate end

file (GLOB APP_TS_FILES ../translations/*.ts)
#foreach(i ${APP_TS_FILES})
#    message(${i})
#    endforeach()
qt5_create_translation(APP_QM_FILES ${APP_TS_FILES} ${APP_QM_FILES})

#INCLUDE(${QT_USE_FILE})
#ADD_DEFINITIONS(${QT_DEFINITIONS})

#9.链接编译的lib
link_directories(compressor-lib compressor-interface)
link_directories(compressor-lib compressor-ChardetDetector)


#10.生成可执行程序
add_executable(${EXE_NAME}  ${c_files} ${h_files} ${QRC_MOC} ${APP_TS_FILES} ${APP_QM_FILES})



set_target_properties(${EXE_NAME} PROPERTIES AUTOMOC TRUE)
qt5_use_modules(${EXE_NAME}  Core  Widgets  Gui )

target_include_directories(${EXE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source/inc )
target_include_directories(${EXE_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/source/inc/openwithdialog )
#foreach(f ${LIBS_INCLUDE_DIRS})
#message(${f})
#endforeach()


target_link_libraries(${EXE_NAME}  ${LIBS_INCLUDE_DIRS})
#message(${LIBS_INCLUDE_DIRS})
target_link_libraries(${EXE_NAME}  Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus Qt5::Svg  KF5::Codecs)


target_link_libraries(${EXE_NAME}  compressor-interface)
target_link_libraries(${EXE_NAME}  compressor-ChardetDetector)



set(CMAKE_INSTALL_PREFIX /usr)


install(FILES ../output/deepin-compressor
        DESTINATION /usr/bin/
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES  deepin-compressor.desktop
        DESTINATION /usr/share/applications/
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

file(GLOB INSTALL_QM_FILES ../translations/*.qm)
install(FILES ${INSTALL_QM_FILES} 
	DESTINATION /usr/share/deepin-compressor/translations)


# for i in `ls *.desktop`; do echo "install(FILES ${i} DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)";done

install(FILES deepin-compress.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-compress-multi.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES deepin-compressor.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES deepin-compress-to_7z.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES deepin-compress-to_zip.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompress.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompresshere.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompresshere-multi.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompresshere-split.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompresshere-split_multi.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES deepin-decompressmkdir.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
#install(FILES deepin-decompressmkdir-split.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompress-multi.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompress-split.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)
install(FILES deepin-decompress-split_multi.desktop DESTINATION /usr/share/deepin/dde-file-manager/oem-menuextensions/)

install(FILES assets/icons/deepin/builtin/icons/deepin-compressor.svg DESTINATION /usr/share/icons/hicolor/scalable/apps/)
install(FILES assets/mimetype/deepin-compressor.xml DESTINATION /usr/share/mime/packages/)

