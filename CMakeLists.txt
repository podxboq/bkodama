project(bkodama)

#paranoid?
cmake_minimum_required(VERSION 2.6.2)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debugfull)
add_definitions(-DQT_STRICT_ITERATORS)
SET(CMAKE_VERBOSE_MAKEFILE 1)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wsign-compare -Wpointer-arith -Wmissing-declarations -Wpacked -Wswitch-enum -Wstrict-aliasing=2 -Winit-self -Wunsafe-loop-optimizations -Wno-missing-field-initializers -Wno-unused-parameter -Wno-attributes -Wno-long-long -Winline")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})

add_subdirectory(data)
add_subdirectory(src)

macro_display_feature_log()
