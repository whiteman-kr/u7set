TARGET = Qtkeychain

TEMPLATE = lib
CONFIG += staticlib

include(../../compiler.pri)

CONFIG += warn_off

# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../s../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

unix:!android:!macx:!ios {
    packagesExist(libsecret-1) {
    }
    else {
	!build_pass:error("Libsecret package is not installed!")
    }
}

include(./qtkeychain.pri)
