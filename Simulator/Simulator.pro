QT       -= gui
QT		 += xml qml core concurrent network

TARGET = Simulator
TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
include(../warnings.pri)

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

SOURCES += \
    ../lib/ConnectionsInfo.cpp \
    ../lib/LmDescription.cpp \
	../lib/LanControllerInfo.cpp \
	../lib/LogicModulesInfo.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/SoftwareXmlReader.cpp \
	../lib/AppSignalManager.cpp \
	../lib/Tuning/TuningSignalManager.cpp \
	../lib/Tuning/TuningSignalState.cpp \
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
	../lib/AppSignalManager.h \
	../lib/IAppSignalManager.h \
	../lib/Tuning/ITuningSignalManager.h \
	../lib/Tuning/TuningSignalManager.h \
	../lib/Tuning/TuningSignalState.h \
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
