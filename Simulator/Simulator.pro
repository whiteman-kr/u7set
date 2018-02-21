#-------------------------------------------------
#
# Project created by QtCreator 2017-11-10T10:13:51
#
#-------------------------------------------------

QT       -= gui
QT		 += xml qml concurrent

TARGET = Simulator
TEMPLATE = lib

CONFIG += staticlib

CONFIG += c++17					# C++17 support is enabled.
win32:QMAKE_CXXFLAGS += /std:c++17

CONFIG += warn_on				# The compiler should output as many warnings as possible. If warn_off is also specified, the last one takes effect.


PRECOMPILED_HEADER = Stable.h

# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}

CONFIG(debug, debug|release) {
    OBJECTS_DIR = debug
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
    OBJECTS_DIR = release
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../lib/LmDescription.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Types.cpp \
    ../lib/ModuleFirmware.cpp \
    Simulator.cpp \
    ../lib/Crc.cpp \
    SimOutput.cpp \
    SimRam.cpp \
    SimLmModel.cpp \
    SimEeprom.cpp \
    SimSubsystem.cpp \
    SimComponent.cpp \
    SimDeviceEmulator.cpp

HEADERS += \
    Stable.h \
    ../lib/LmDescription.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/PropertyObject.h \
    ../lib/ProtoSerialization.h \
    ../lib/Types.h \
    ../lib/ModuleFirmware.h \
    Simulator.h \
    ../lib/Crc.h \
    SimOutput.h \
    SimRam.h \
    SimLmModel.h \
    SimEeprom.h \
    SimSubsystem.h \
    SimComponent.h \
    SimDeviceEmulator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


## VFrame30 library
## $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
##
#win32 {
#    CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
#    CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
#}
#unix {
#    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
#    CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
#}

#INCLUDEPATH += ../VFrame30
#DEPENDPATH += ../VFrame30

#protobuf
#
win32 {
    LIBS += -L$$DESTDIR -lprotobuf

    INCLUDEPATH += ./../Protobuf
}
unix {
    LIBS += -lprotobuf
}

DISTFILES += \
    Scripts/LM1_SR01_SIM.ts \
    Scripts/tsconfig.json \
    Scripts/build.bat \
    Scripts/out/LM1_SR01_SIM.js

