project(bkodama)

cmake_minimum_required(VERSION 2.8.12)


set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.4.1")
set(KF5_MIN_VERSION "5.9.0")
set(CMAKE_INSTALL_PREFIX /usr)
set(KDE_INSTALL_LIBDIR lib64/qt5)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Quick QuickWidgets)
# CMAKE EXTRA MODULES
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
        Plasma PlasmaQuick WindowSystem ConfigWidgets XmlGui GlobalAccel I18n)

include(KDECMakeSettings)
include(KDECompilerSettings)
include(KDEInstallDirs)
include(ECMPackageConfigHelpers)

add_subdirectory(engine)
add_subdirectory(plasmoid)

file(GLOB_RECURSE SRCS *.qml *.js main.xml *.desktop *.operations)
add_custom_target(contents SOURCES ${SRCS})
