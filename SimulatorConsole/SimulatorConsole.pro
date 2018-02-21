QT -= gui
QT += xml qml

CONFIG += c++11 console
CONFIG -= app_bundle

win32:QMAKE_CXXFLAGS += /std:c++17

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# DESTDIR
# If you see somewhere 'LNK1146: no argument specified with option '/LIBPATH:' then most likely you have not added this section to a project file
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}
# /DESTDIR
#

SOURCES += main.cpp

# Simulator Lib
#
INCLUDEPATH += $$PWD/../Simulator
DEPENDPATH += $$PWD/../Simulator

win32 {
    LIBS += -L$$DESTDIR -lSimulator

    CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/debug/Simulator.lib
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/release/Simulator.lib
}
unix {
    LIBS += -lSimulator
}


# VFrame30 library
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
    CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
    CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

#protobuf
#
win32 {
    LIBS += -L$$DESTDIR -lprotobuf
    INCLUDEPATH += ./../Protobuf
}
unix {
    LIBS += -lprotobuf
}

