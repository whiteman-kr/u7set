TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += $$PWD/libgit2headers
win32:LIBS += $$PWD/git2.lib

# DESTDIR
#
win32 {
        DESTDIR = ../
}
unix {
        DESTDIR = ../bin_unix
}

#c++11 support for GCC
#
*g++:QMAKE_CXXFLAGS += -std=c++11

# libgit2
unix:LIBS += -lgit2

