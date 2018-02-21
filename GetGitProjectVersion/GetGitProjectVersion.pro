TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp


# libgit2
#
win32{
        contains(QMAKE_TARGET.arch, x86_64){
                LIBS += $$PWD/../bin_Win64/git2.lib
        }
        else{
                LIBS += $$PWD/../bin_Win32/git2.lib
        }

        INCLUDEPATH += $$PWD/libgit2headers
}


#win32:INCLUDEPATH += $$PWD/libgit2headers
#win32:LIBS += $$PWD/git2.lib

# DESTDIR
#
win32 {
        DESTDIR = ../

        contains(QMAKE_TARGET.arch, x86_64){
                DESTDIR = ../bin_Win64/
        }
        else{
                DESTDIR = ../bin_Win32/
        }

}
unix {
        DESTDIR = ../bin_unix
}

#c++11 support for GCC
#
gcc:QMAKE_CXXFLAGS += -std=c++11
win32:QMAKE_CXXFLAGS += /std:c++17

# libgit2
unix:LIBS += -lgit2

