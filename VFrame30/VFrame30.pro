#-------------------------------------------------
#
# Project created by QtCreator 2012-10-02T18:35:47
#
#-------------------------------------------------
TARGET = VFrame30

TEMPLATE = lib
QT += widgets qml xml

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD

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
	../lib/TypesAndEnums.h \
	Configuration.h \
    VFrame30.h \
    ../lib/StreamedData.h \
    ../lib/ProtoSerialization.h \
    ../lib/CUtils.h \
	MonitorSchema.h \
    ../lib/DbStruct.h \
    Afb.h \
    ../lib/DebugInstCounter.h \
	../lib/PropertyObject.h \
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
    BaseSchemaWidget.h \
    SchemaPoint.h \
    ../lib/Types.h \
    PropertyNames.h \
    ../lib/AppSignalState.h \
    ../lib/AppSignalManager.h \
    ../lib/Signal.h \
    ../lib/XmlHelper.h \
    ../lib/DeviceObject.h \
    SchemaItemConnection.h \
    ../lib/HostAddressPort.h \
    ../lib/Factory.h \
    UfbSchema.h \
    SchemaItemUfb.h \
    SchemaItemTerminator.h \
    MacrosExpander.h \
    Session.h \
    SchemaItemControl.h \
    SchemaItemPushButton.h \
    SchemaItemLineEdit.h \
    ../lib/Tuning/TuningController.h \
    SchemaItemValue.h \
    ../lib/Tuning/TuningSignalState.h

SOURCES += \
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
    ../lib/StreamedData.cpp \
    ../lib/ProtoSerialization.cpp \
	MonitorSchema.cpp \
    ../lib/DbStruct.cpp \
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
    BaseSchemaWidget.cpp \
    SchemaPoint.cpp \
    ../lib/Types.cpp \
    PropertyNames.cpp \
    ../lib/AppSignalState.cpp \
    ../lib/AppSignalManager.cpp \
    ../lib/Signal.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/DeviceObject.cpp \
    SchemaItemConnection.cpp \
    ../lib/HostAddressPort.cpp \
    UfbSchema.cpp \
    SchemaItemUfb.cpp \
    SchemaItemTerminator.cpp \
    MacrosExpander.cpp \
    Session.cpp \
    SchemaItemControl.cpp \
    SchemaItemPushButton.cpp \
    SchemaItemLineEdit.cpp \
    ../lib/Tuning/TuningController.cpp \
    SchemaItemValue.cpp \
    ../lib/Tuning/TuningSignalState.cpp

DEFINES += VFRAME30LIB_LIBRARY
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

#Optimization flags
#
win32 {

    QMAKE_CXXFLAGS += -wd4275
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

# Protobuf
#
#!include(protobuf.pri) {
#	error("Couldn't find the protobuf.pri file!")
#}

