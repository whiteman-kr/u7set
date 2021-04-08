QT += testlib xml qml core concurrent network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

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
    SimProfilesTest.cpp \
    SimRamTests.cpp \
    main.cpp


# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''


#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../../Protobuf

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
    SimProfilesTest.h \
    SimRamTests.h

RESOURCES += \
    Resources.qrc

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lOnlineLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lOnlineLib
}
