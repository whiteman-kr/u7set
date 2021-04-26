QT       -= gui
QT		 += xml qml core concurrent network

TARGET = Simulator
TEMPLATE = lib
CONFIG += staticlib

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

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
    ../lib/ConnectionsInfo.cpp \
    ../lib/LmDescription.cpp \
	../lib/LogicModulesInfo.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/SoftwareXmlReader.cpp \
	../lib/AppSignalManager.cpp \
	../lib/AppSignal.cpp \
	../lib/AppSignalParam.cpp \
	../lib/Tuning/TuningSignalManager.cpp \
	../lib/Tuning/TuningSignalState.cpp \
	../lib/TuningValue.cpp \
	../lib/AppSignalProperties.cpp	\
    SimAppDataLanInterface.cpp \
    SimAppDataTransmitter.cpp \
    SimCommandProcessor_LM5_LM6.cpp \
    SimConnections.cpp \
    SimDiagDataLanInterface.cpp \
    SimLanInterface.cpp \
    SimLans.cpp \
    SimProfiles.cpp \
    SimScopedLog.cpp \
    SimScriptConnection.cpp \
    SimScriptDevUtils.cpp \
	SimScriptLmDescription.cpp \
    SimScriptLogicModule.cpp \
    SimScriptRamAddress.cpp \
    SimScriptSignal.cpp \
    SimScriptSimulator.cpp \
    SimSoftware.cpp \
    SimTuningLanInterface.cpp \
    SimTuningRecord.cpp \
    SimTuningServiceCommunicator.cpp \
    Simulator.cpp \
    SimRam.cpp \
    SimEeprom.cpp \
    SimSubsystem.cpp \
    SimDeviceEmulator.cpp \
    SimControl.cpp \
    SimTimeController.cpp \
    SimAppSignalManager.cpp \
    SimTuningSignalManager.cpp \
    SimAfb.cpp \
    SimLogicModule.cpp \
    SimCommandProcessor.cpp \
    SimException.cpp \
	SimOverrideSignals.cpp

HEADERS += \
	Stable.h \
    ../lib/ConnectionsInfo.h \
	../UtilsLib/ILogFile.h \
	../lib/LanControllerInfo.h \
	../lib/LogicModulesInfo.h \
    ../lib/SoftwareSettings.h \
    ../lib/SoftwareXmlReader.h \
	../Proto/serialization.pb.h \
	../lib/LmDescription.h \
	../CommonLib/PropertyObject.h \
	../lib/AppSignalManager.h \
	../lib/AppSignal.h \
	../lib/AppSignalParam.h \
	../lib/IAppSignalManager.h \
	../lib/Tuning/ITuningSignalManager.h \
	../lib/Tuning/TuningSignalManager.h \
	../lib/Tuning/TuningSignalState.h \
	../lib/TuningValue.h \
	../lib/AppSignalProperties.h \
    SimAppDataLanInterface.h \
    SimAppDataTransmitter.h \
    SimCommandProcessor_LM5_LM6.h \
    SimConnections.h \
    SimDiagDataLanInterface.h \
    SimLanInterface.h \
    SimLans.h \
    SimProfiles.h \
    SimScopedLog.h \
    SimScriptConnection.h \
    SimScriptDevUtils.h \
	SimScriptLmDescription.h \
    SimScriptLogicModule.h \
    SimScriptRamAddress.h \
    SimScriptSignal.h \
    SimScriptSimulator.h \
    SimSoftware.h \
    SimTuningLanInterface.h \
    SimTuningRecord.h \
    SimTuningServiceCommunicator.h \
    Simulator.h \
    SimRam.h \
    SimEeprom.h \
    SimSubsystem.h \
    SimDeviceEmulator.h \
    SimControl.h \
    SimTimeController.h \
    SimAppSignalManager.h \
    SimTuningSignalManager.h \
    SimAfb.h \
    SimLogicModule.h \
    SimCommandProcessor.h \
    SimException.h \
    SimOverrideSignals.h \

# Protobuf
#
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    SimProjectTests.js
