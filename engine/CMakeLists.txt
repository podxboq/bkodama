project(bkodama)

set(bkodama_SRCS bkodama.cpp circleeffectdrawer.cpp)

ki18n_wrap_ui(bkodama_SRCS bkodamaConfig.ui)

add_library(plasma_applet_bkodama ${bkodama_SRCS})

target_link_libraries(plasma_applet_bkodama ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} )

install(TARGETS plasma_applet_bkodama
        DESTINATION ${PLUGIN_INSTALL_DIR})

