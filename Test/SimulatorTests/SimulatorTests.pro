QT += testlib xml qml core concurrent network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# C++17 support is enabled.
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17

CONFIG += warn_off				# The compiler should output as many warnings as possible. If warn_off is also specified, the last one takes effect.

# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}


SOURCES +=  \
    SimCommandTest_LM5_LM6.cpp \
    SimRamTests.cpp \
    main.cpp


#protobuf
#
win32 {
    LIBS += -L$$DESTDIR -lprotobuf
	INCLUDEPATH += ./../../Protobuf
}
unix {
    LIBS += -lprotobuf
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

# Simulator Lib
#
INCLUDEPATH += $$PWD/../../Simulator
DEPENDPATH += $$PWD/../../Simulator

win32 {
    LIBS += -L$$DESTDIR -lSimulator

    CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../bin/debug/Simulator.lib
	CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../bin/release/Simulator.lib
}
unix {
    LIBS += -lSimulator
}

HEADERS += \
    ../../lib/PropertyObject.h \
    SimCommandTest_LM5_LM6.h \
    SimRamTests.h

RESOURCES += \
    Resources.qrc
