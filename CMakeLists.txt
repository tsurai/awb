project(awb)
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WEBKITGTK REQUIRED webkitgtk-3.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GNUTLS REQUIRED gnutls)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
include_directories(${WEBKITGTK_INCLUDE_DIRS})
link_directories(${WEBKITGTK_LIBRARY_DIRS})
include_directories(${GNUTLS_INCLUDE_DIRS})
link_directories(${GNUTLS_LIBRARY_DIRS})
include_directories(${SQLITE3_INCLUDE_DIRS})
link_directories(${SQLITE3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

SET(awb_SOURCES
    src/adblock.h
    src/adblock.c
    src/commands.h
    src/commands.c
    src/database.h
    src/database.c
    src/binds.h
    src/binds.c
    src/factory.h
    src/factory.c
    src/gui.h
    src/gui.c
    src/awb.h
    src/awb.c)

SET(CC "clang")
SET(CMAKE_CXX_FLAGS "-std=c90 -g -Wall -Wextra")
add_definitions(${CMAKE_CXX_FLAGS})

add_executable(${PROJECT_NAME} ${awb_SOURCES})
target_link_libraries(awb ${GTK3_LIBRARIES})
target_link_libraries(awb ${WEBKITGTK_LIBRARIES})
target_link_libraries(awb ${GNUTLS_LIBRARIES})
target_link_libraries(awb ${SQLITE3_LIBRARIES})
