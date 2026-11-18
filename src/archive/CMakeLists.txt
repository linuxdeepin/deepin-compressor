add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/api ${CMAKE_BINARY_DIR}/archive-api)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/singlefile ${CMAKE_BINARY_DIR}/own-singlefile)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/libarchive ${CMAKE_BINARY_DIR}/own-libarchive)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/libarchive-ro ${CMAKE_BINARY_DIR}/own-libarchive-ro)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/minizip ${CMAKE_BINARY_DIR}/own-minizip)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/pigz ${CMAKE_BINARY_DIR}/own-pigz)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/libzip ${CMAKE_BINARY_DIR}/own-libzip)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/cli7z ${CMAKE_BINARY_DIR}/own-cli7z)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/clirar ${CMAKE_BINARY_DIR}/own-clirar)

if((CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm|x86_64|amd64") AND (QT_VERSION_MAJOR EQUAL 6))
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/plugins/clipzip ${CMAKE_BINARY_DIR}/own-clipzip)
endif()
