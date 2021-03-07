project(bkodama)

install(FILES plasma-applet-bkodama.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES
        sitting01-test.svg
        spin00-test.svg
        spin01-test.svg
        spin02-test.svg
        spin03-test.svg
        spin04-test.svg
        spin05-test.svg
        stand00-test.svg
        stand01-test.svg
        stand02-test.svg
        stand03-test.svg
        walk00-test.svg
        walk01-test.svg
        walk02-test.svg
        walk03-test.svg
        walk04-test.svg
        walk05-test.svg
        walk06-test.svg
        special00-test.svg
        special01-test.svg
        special02-test.svg
        special03-test.svg
        special04-test.svg
        specialfail01-test.svg
        specialfail02-test.svg
        specialfail03-test.svg
        specialfail04-test.svg
        specialfail05-test.svg
        specialfail06-test.svg
        specialfail07-test.svg
        specialfail08-test.svg
        specialfail09-test.svg
        head-spin3.ogg
        DESTINATION ${DATA_INSTALL_DIR}/bkodama/)

kde4_install_icons(${ICON_INSTALL_DIR})
