#-------------------------------------------------
#
# Project created by QtCreator 2017-11-10T10:13:51
#
#-------------------------------------------------

QT       -= gui
QT		 += xml qml core concurrent network

TARGET = Simulator
TEMPLATE = lib
CONFIG += staticlib

# C++17 support is enabled.
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17

# Optimization flags
#
win32 {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

CONFIG += warn_on				# The compiler should output as many warnings as possible. If warn_off is also specified, the last one takes effect.

CONFIG += precompile_header
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

unix {
    target.path = /usr/lib
    INSTALLS += target
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
    ../lib/Address16.cpp \
    ../lib/AppSignalStateFlags.cpp \
    ../lib/ConnectionsInfo.cpp \
    ../lib/LmDescription.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Types.cpp \
    ../lib/ModuleFirmware.cpp \
    SimCommandProcessor_LM5_LM6.cpp \
    SimConnections.cpp \
    SimScriptDevUtils.cpp \
    SimScriptLogicModule.cpp \
    SimScriptRamAddress.cpp \
    SimScriptSignal.cpp \
    SimScriptSimulator.cpp \
    Simulator.cpp \
    ../lib/Crc.cpp \
    SimOutput.cpp \
    SimRam.cpp \
    SimEeprom.cpp \
    SimSubsystem.cpp \
    SimDeviceEmulator.cpp \
    SimControl.cpp \
    SimTimeController.cpp \
    SimAppSignalManager.cpp \
    SimTuningSignalManager.cpp \
    ../lib/AppSignalManager.cpp \
    ../lib/Signal.cpp \
    ../lib/AppSignal.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/TuningValue.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    SimAfb.cpp \
    SimLogicModule.cpp \
    SimCommandProcessor.cpp \
    SimException.cpp \
    SimOverrideSignals.cpp \
    ../lib/SignalProperties.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp

HEADERS += \
    ../Builder/CfgFiles.h \
    ../lib/Address16.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/ConnectionsInfo.h \
    ../lib/Hash.h \
    SimCommandProcessor_LM5_LM6.h \
    SimConnections.h \
    SimScriptDevUtils.h \
    SimScriptLogicModule.h \
    SimScriptRamAddress.h \
    SimScriptSignal.h \
    SimScriptSimulator.h \
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
    SimEeprom.h \
    SimSubsystem.h \
    SimDeviceEmulator.h \
    SimControl.h \
    SimTimeController.h \
    SimAppSignalManager.h \
    SimTuningSignalManager.h \
    ../lib/AppSignalManager.h \
    ../lib/Signal.h \
    ../lib/AppSignal.h \
    ../lib/IAppSignalManager.h \
    ../lib/Tuning/ITuningSignalManager.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/TuningValue.h \
    ../lib/XmlHelper.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    SimAfb.h \
    SimLogicModule.h \
    SimCommandProcessor.h \
    SimException.h \
    SimOverrideSignals.h \
    ../lib/SignalProperties.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h

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
    Scripts/out/LM1_SR01_SIM.js \
    SimProjectTests.js




