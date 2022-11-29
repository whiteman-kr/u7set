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

include(./qtkeychain.pri)
