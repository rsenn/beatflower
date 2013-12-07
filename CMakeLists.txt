project(beatflower)

cmake_minimum_required(VERSION 2.4)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

#include(FindGTK)
include(FindSDL)

find_program(CMAKE_GTK_CONFIG gtk-config ../gtk/bin ../../gtk/bin)
exec_program("${CMAKE_GTK_CONFIG}" ARGS "--cflags" OUTPUT_VARIABLE GTK_COMPILE_FLAGS)
exec_program("${CMAKE_GTK_CONFIG}" ARGS "--libs" OUTPUT_VARIABLE GTK_LINK_FLAGS)

find_program(CMAKE_XMMS_CONFIG xmms-config)
exec_program("${CMAKE_XMMS_CONFIG}" ARGS "--cflags" OUTPUT_VARIABLE XMMS_COMPILE_FLAGS)
exec_program("${CMAKE_XMMS_CONFIG}" ARGS "--libs" OUTPUT_VARIABLE XMMS_LINK_FLAGS)
exec_program("${CMAKE_XMMS_CONFIG}" ARGS "--visualization-plugin-dir" OUTPUT_VARIABLE XMMS_VISUALIZATION_PLUGIN_DIR)

message(STATUS "GTK_COMPILE_FLAGS = ${GTK_COMPILE_FLAGS} GTK_LINK_FLAGS = ${GTK_LINK_FLAGS}")

set(DEFS PACKAGE="beatflower" VERSION="1.0")
#add_definitions(${DEFS})

#include_directories(${GTK_glib_INCLUDE_PATH} ${GTK_glibconfig_INCLUDE_PATH} ${GTK_gtk_INCLUDE_PATH})
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

#target_link_libraries(beatflower ${GTK_gtk_LIBRARY})
set_target_properties(beatflower PROPERTIES
  COMPILE_DEFINITIONS "${DEFS}"
  COMPILE_FLAGS "${XMMS_COMPILE_FLAGS}"
  LINK_FLAGS "${XMMS_LINK_FLAGS}")
