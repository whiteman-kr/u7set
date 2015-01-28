TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

# DESTDIR
#
win32 {
        CONFIG(debug, debug|release): DESTDIR = $$PWD/../bin/debug
        CONFIG(release, debug|release): DESTDIR = $$PWD/../bin/release
}
unix {
        CONFIG(debug, debug|release): DESTDIR = $$PWD/../bin_unix/debug
        CONFIG(release, debug|release): DESTDIR = $$PWD/../bin_unix/release
}

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

# libgit2
LIBS += -lgit2

