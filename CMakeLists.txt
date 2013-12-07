project(beatflower)

cmake_minimum_required(VERSION 2.4)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

#include(FindGTK)
include(FindSDL)

find_program(CMAKE_GTK_CONFIG gtk-config ../gtk/bin ../../gtk/bin)
exec_program("${CMAKE_GTK_CONFIG}" ARGS "--cflags" OUTPUT_VARIABLE GTK_COMPILE_FLAGS)
exec_program("${CMAKE_GTK_CONFIG}" ARGS "--libs" OUTPUT_VARIABLE GTK_LINK_FLAGS)

message(STATUS "GTK_COMPILE_FLAGS = ${GTK_COMPILE_FLAGS} GTK_LINK_FLAGS = ${GTK_LINK_FLAGS}")

#set(GTK_COMPILE_FLAGS "`${CMAKE_GTK_CONFIG} --cflags`")
#set(GTK_LINK_FLAGS "`${CMAKE_GTK_CONFIG} --libs --gl-libs`")

add_definitions(-DPACKAGE=\\"beatflower\\")
add_definitions(-DVERSION=\\"1.0\\")

include_directories(${GTK_glib_INCLUDE_PATH} ${GTK_glibconfig_INCLUDE_PATH} ${GTK_gtk_INCLUDE_PATH})
include_directories(${SDL_INCLUDE_DIR})

set(SOURCES
    beatflower.c
    beatflower.h
    beatflower_xmms.h
    beatflower_xmms_config.c
    beatflower_xmms_plugin.c
    beatflower_xmms_settings.c
)

add_library(beatflower SHARED ${SOURCES})

target_link_libraries(beatflower ${GTK_gtk_LIBRARY})
set_target_properties(beatflower PROPERTIES COMPILE_FLAGS "${GTK_COMPILE_FLAGS}" LINK_FLAGS "${GTK_LINK_FLAGS}")
