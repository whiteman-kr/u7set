QT       += qml sql xml widgets concurrent

TARGET = Builder
TEMPLATE = lib
CONFIG += staticlib

# c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17
win32:QMAKE_CXXFLAGS += /analyze		# Static code analyze

# Warning level
#
gcc:CONFIG += warn_on

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'


CONFIG(debug, debug|release): DEFINES += Q_DEBUG

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

DEFINES += IS_BUILDER

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../lib/ClientBehavior.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DomXmlHelper.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/LogicModulesInfo.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbController.cpp \
    ../lib/DbProgress.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DbWorker.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Connection.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../TuningService/TuningSource.cpp \
    ../lib/LogicModuleSet.cpp \
    ../lib/ModuleFirmware.cpp \
    ../lib/Signal.cpp \
    ../lib/Subsystem.cpp \
    ../lib/Types.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/TuningValue.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/DataSource.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/Queue.cpp \
    ../lib/WUtils.cpp \
    ../lib/CsvFile.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/AppSignal.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/LmDescription.cpp \
    ../lib/SocketIO.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../lib/ConnectionsInfo.cpp \
    Builder.cpp \
    IssueLogger.cpp \
    BuildWorkerThread.cpp \
    BuildResultWriter.cpp \
    Loopbacks.cpp \
    ModuleFirmwareWriter.cpp \
    Parser.cpp \
    SignalSet.cpp \
    Busses.cpp \
    OptoModule.cpp \
    RawDataDescription.cpp \
    SignalsHeap.cpp \
    UalItems.cpp \
    ApplicationLogicCode.cpp \
    LmMemoryMap.cpp \
    ConfigurationBuilder.cpp \
    ApplicationLogicCompiler.cpp \
    ModuleLogicCompiler.cpp \
    MemWriteMap.cpp \
    SoftwareCfgGenerator.cpp \
    AppDataServiceCfgGenerator.cpp \
    ModulesRawData.cpp \
    DiagDataServiceCfgGenerator.cpp \
    MonitorCfgGenerator.cpp \
    TuningClientCfgGenerator.cpp \
    TuningServiceCfgGenerator.cpp \
    ConfigurationServiceCfgGenerator.cpp \
    ArchivingServiceCfgGenerator.cpp \
    MetrologyCfgGenerator.cpp \
    LmDescriptionSet.cpp \
    BdfFile.cpp \
    FbParamCalculation.cpp \
    TuningBuilder.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/Address16.cpp \
    ../lib/Times.cpp \
    ../lib/DataProtocols.cpp \
	../lib/DbProgressDialog.cpp \
    ../lib/MetrologySignal.cpp \
    RunOrder.cpp \
    TestClientCfgGenerator.cpp \
    ../lib/UnitsConvertor.cpp \
    ../lib/UnitsConvertorTable.cpp \
    Context.cpp \
    ../lib/AppSignalStateFlags.cpp \
    ../lib/ComparatorSet.cpp

HEADERS += \
    ../lib/ClientBehavior.h \
    ../lib/ConstStrings.h \
    ../lib/DeviceHelper.h \
    ../lib/DomXmlHelper.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/LogicModulesInfo.h \
    ../lib/OutputLog.h \
    ../lib/DbController.h \
    ../lib/DbProgress.h \
    ../lib/DbStruct.h \
    ../lib/DbWorker.h \
    ../lib/DeviceObject.h \
    ../lib/DbObjectStorage.h \
    ../lib/Connection.h \
    ../TuningService/TuningSource.h \
    ../TuningService/TuningDataStorage.h \
    ../TuningService/TuningSource.h \
    ../lib/CommonTypes.h \
    ../lib/LogicModuleSet.h \
    ../lib/PropertyObject.h \
    ../lib/ModuleFirmware.h \
    ../lib/Signal.h \
    ../lib/Subsystem.h \
    ../lib/Types.h \
    ../lib/ProtoSerialization.h \
    ../lib/TuningValue.h \
    ../lib/XmlHelper.h \
    ../lib/SignalProperties.h \
    ../lib/Crc.h \
    ../lib/Hash.h \
    ../lib/HostAddressPort.h \
    ../lib/DataSource.h \
	../lib/SoftwareSettings.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/CsvFile.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/AppSignal.h \
    ../lib/BuildInfo.h \
    ../lib/LmDescription.h \
    ../lib/SocketIO.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../VFrame30/VFrame30Lib_global.h \
    ../lib/ConnectionsInfo.h \
	Builder.h \
    Loopbacks.h \
    SignalsHeap.h \
    Stable.h \
    IssueLogger.h \
    BuildWorkerThread.h \
    BuildResultWriter.h \
    ModuleFirmwareWriter.h \
    Parser.h \
    SignalSet.h \
    Busses.h \
    OptoModule.h \
    RawDataDescription.h \
    UalItems.h \
    ApplicationLogicCode.h \
    LmMemoryMap.h \
    TuningBuilder.h \
    ConfigurationBuilder.h \
    ApplicationLogicCompiler.h \
    ModuleLogicCompiler.h \
    MemWriteMap.h \
    SoftwareCfgGenerator.h \
    AppDataServiceCfgGenerator.h \
    ModulesRawData.h \
    DiagDataServiceCfgGenerator.h \
    MonitorCfgGenerator.h \
    TuningClientCfgGenerator.h \
    TuningServiceCfgGenerator.h \
    ConfigurationServiceCfgGenerator.h \
    ArchivingServiceCfgGenerator.h \
    MetrologyCfgGenerator.h \
    LmDescriptionSet.h \
    BdfFile.h \
    ../lib/Tuning/TuningSignalState.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/Address16.h \
    ../lib/Times.h \
    ../lib/DataProtocols.h \
    ../lib/DbProgressDialog.h \
    ../lib/MetrologySignal.h \
    RunOrder.h \
    TestClientCfgGenerator.h \
    ../lib/UnitsConvertor.h \
    ../lib/UnitsConvertorTable.h \
    Context.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/ComparatorSet.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


# protobuf
#
win32 {
    LIBS += -L$$DESTDIR -lprotobuf
    INCLUDEPATH += ./../Protobuf
}
unix {
    LIBS += -lprotobuf
}

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto \
    ../Test/CompilerTests/CompilerTests.js
