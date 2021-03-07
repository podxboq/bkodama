project(bkodama)

cmake_minimum_required(VERSION 3.16)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Quick QuickWidgets)
# Use Extra CMake Modules (ECM) for common functionality.
# See http://api.kde.org/ecm/manual/ecm.7.html
# and http://api.kde.org/ecm/manual/ecm-kde-modules.7.html
find_package(ECM REQUIRED NO_MODULE)
# Needed by find_package(KF5Plasma) below.
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
if(CMAKE_BUILD_TYPE_TOLOWER MATCHES debugfull)
add_definitions(-DQT_STRICT_ITERATORS)
SET(CMAKE_VERBOSE_MAKEFILE 1)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wsign-compare -Wpointer-arith -Wmissing-declarations -Wpacked -Wswitch-enum -Wstrict-aliasing=2 -Winit-self -Wunsafe-loop-optimizations -Wno-missing-field-initializers -Wno-unused-parameter -Wno-attributes -Wno-long-long -Winline")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

add_subdirectory(plasmoid)
