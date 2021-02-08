#-------------------------------------------------
#
# Project created by QtCreator 2012-10-02T18:35:47
#
#-------------------------------------------------
TARGET = VFrame30

TEMPLATE = lib
QT += widgets qml xml svg

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		# CONFIG += c++17 has no effect yet
win32:QMAKE_CXXFLAGS += /analyze		# Static code analyze

# Warning level
#
gcc:CONFIG += warn_on

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'


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


OTHER_FILES += \
    ../Proto/proto_compile.bat \
    ../Proto/serialization.proto \
    ../Proto/proto_compile.sh

HEADERS += VFrame30Lib_global.h \
    ../Proto/serialization.pb.h \
    ../Proto/trends.pb.h \
    ../lib/Address16.h \
    ../lib/ClientBehavior.h \
    ../lib/ComparatorSet.h \
    ../lib/ILogFile.h \
    Indicator.h \
    IndicatorArrowIndicator.h \
    IndicatorHistogramVert.h \
    LogController.h \
    SchemaItemFrame.h \
    SchemaItemIndicator.h \
    Stable.h \
    Settings.h \
    PosRectImpl.h \
    PosLineImpl.h \
    PosConnectionImpl.h \
    FontParam.h \
    FblItemRect.h \
    FblItemLine.h \
    FblItem.h \
    DrawParam.h \
    Print.h \
    VFrame30Library.h \
    HorzVertLinks.h \
	Configuration.h \
    VFrame30.h \
	MonitorSchema.h \
    Afb.h \
    Schema.h \
    LogicSchema.h \
    WiringSchema.h \
    DiagSchema.h \
    SchemaLayer.h \
    SchemaView.h \
    SchemaItem.h \
    SchemaItemAfb.h \
    SchemaItemConst.h \
    SchemaItemLine.h \
    SchemaItemLink.h \
    SchemaItemPath.h \
    SchemaItemRect.h \
    SchemaItemSignal.h \
    SchemaItemControl.h \
    SchemaItemPushButton.h \
    SchemaItemLineEdit.h \
    SchemaItemValue.h \
    BaseSchemaWidget.h \
    SchemaPoint.h \
    PropertyNames.h \
    SchemaItemConnection.h \
    UfbSchema.h \
    SchemaItemUfb.h \
    SchemaItemTerminator.h \
    MacrosExpander.h \
    Session.h \
    ../lib/TypesAndEnums.h \
    ../lib/ProtoSerialization.h \
    ../lib/CUtils.h \
    ../lib/DebugInstCounter.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/AppSignalManager.h \
    ../lib/HostAddressPort.h \
    ../lib/Factory.h \
    ../lib/AppSignal.h \
    ../lib/DbStruct.h \
    SchemaItemBus.h \
    Bus.h \
    ClientSchemaWidget.h \
    SchemaManager.h \
    ClientSchemaView.h \
    SchemaItemLoopback.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/ITuningSignalManager.h \
    ../lib/Tuning/ITuningTcpClient.h \
    ../Proto/network.pb.h \
    ../lib/TuningValue.h \
    TuningController.h \
    AppSignalController.h \
    ../lib/Signal.h \
    ../lib/DeviceObject.h \
    ../lib/SignalProperties.h \
    ../lib/XmlHelper.h \
    TuningSchema.h \
    SchemaItemImage.h \
    SchemaItemImageValue.h \
    ImageItem.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h

SOURCES += \
    ../Proto/serialization.pb.cc \
    ../Proto/trends.pb.cc \
    ../lib/Address16.cpp \
    ../lib/ClientBehavior.cpp \
    ../lib/ComparatorSet.cpp \
    Indicator.cpp \
    IndicatorArrowIndicator.cpp \
    IndicatorHistogramVert.cpp \
    LogController.cpp \
    SchemaItemFrame.cpp \
    SchemaItemIndicator.cpp \
    Settings.cpp \
    PosRectImpl.cpp \
    PosLineImpl.cpp \
    PosConnectionImpl.cpp \
    FontParam.cpp \
    FblItemRect.cpp \
    FblItemLine.cpp \
    FblItem.cpp \
    DrawParam.cpp \
    Print.cpp \
    VFrame30Library.cpp \
    HorzVertLinks.cpp \
	Configuration.cpp \
	MonitorSchema.cpp \
    Afb.cpp \
    Schema.cpp \
    LogicSchema.cpp \
    WiringSchema.cpp \
    DiagSchema.cpp \
    SchemaLayer.cpp \
    SchemaView.cpp \
    SchemaItem.cpp \
    SchemaItemAfb.cpp \
    SchemaItemConst.cpp \
    SchemaItemLine.cpp \
    SchemaItemLink.cpp \
    SchemaItemPath.cpp \
    SchemaItemRect.cpp \
    SchemaItemSignal.cpp \
    SchemaItemControl.cpp \
    SchemaItemPushButton.cpp \
    SchemaItemLineEdit.cpp \
    SchemaItemValue.cpp \
    BaseSchemaWidget.cpp \
    SchemaPoint.cpp \
    PropertyNames.cpp \
    SchemaItemConnection.cpp \
    UfbSchema.cpp \
    SchemaItemUfb.cpp \
    SchemaItemTerminator.cpp \
    MacrosExpander.cpp \
    Session.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/AppSignalManager.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/AppSignal.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    SchemaItemBus.cpp \
    Bus.cpp \
    ClientSchemaWidget.cpp \
    SchemaManager.cpp \
    ClientSchemaView.cpp \
    SchemaItemLoopback.cpp \
    ../Proto/network.pb.cc \
    ../lib/TuningValue.cpp \
    TuningController.cpp \
    AppSignalController.cpp \
    ../lib/Types.cpp \
    ../lib/Signal.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/XmlHelper.cpp \
    TuningSchema.cpp \
    SchemaItemImage.cpp \
    SchemaItemImageValue.cpp \
    ImageItem.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp

DEFINES += VFRAME30LIB_LIBRARY
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


# Optimization flags
#
win32 {
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

# Protobuf
#
#!include(protobuf.pri) {
#	error("Couldn't find the protobuf.pri file!")
#}

