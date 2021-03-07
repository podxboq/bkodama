project(bkodama)

set(bkodama_SRCS bkodama.cpp circleeffectdrawer.cpp)

kde4_add_plugin(plasma_applet_bkodama ${bkodama_SRCS})

target_link_libraries(plasma_applet_bkodama ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} )

install(TARGETS plasma_applet_bkodama
        DESTINATION ${PLUGIN_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})
