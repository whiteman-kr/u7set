QT += widgets qml xml svg

TARGET = VFrame30

TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
include(../warnings.pri)
include(../codecoverage.pri)

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD
INCLUDEPATH += ./../Protobuf

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

unix {
    target.path = /usr/lib
	INSTALLS += target
}

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

HEADERS += \
    ../Proto/serialization.pb.h \
    ../lib/ClientBehavior.h \
    ../lib/ComparatorSet.h \
	../UtilsLib/ILogFile.h \
    Context.h \
    ISchemaViewHistory.h \
    IViewVariables.h \
    Indicator.h \
    IndicatorArrowIndicator.h \
    IndicatorHistogramVert.h \
    IndicatorTrend.h \
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
    VFrame30Library.h \
    HorzVertLinks.h \
	Configuration.h \
	MonitorSchema.h \
    Afb.h \
    Schema.h \
    LogicSchema.h \
    VFrameTools.h \
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
    ../CommonLib/PropertyObject.h \
	../lib/IAppSignalManager.h \
    SchemaItemBus.h \
    Bus.h \
    ClientSchemaWidget.h \
    SchemaManager.h \
    ClientSchemaView.h \
    SchemaItemLoopback.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/ITuningSignalManager.h \
    ../lib/Tuning/ITuningTcpClient.h \
	TuningController.h \
    AppSignalController.h \
    TuningSchema.h \
    SchemaItemImage.h \
    SchemaItemImageValue.h \
    ImageItem.h \
    ../lib/OutputLog.h

SOURCES += \
    Context.cpp \
    Indicator.cpp \
    IndicatorArrowIndicator.cpp \
    IndicatorHistogramVert.cpp \
    IndicatorTrend.cpp \
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
    VFrame30Library.cpp \
    HorzVertLinks.cpp \
	Configuration.cpp \
	MonitorSchema.cpp \
    Afb.cpp \
    Schema.cpp \
    LogicSchema.cpp \
    VFrameTools.cpp \
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
    SchemaItemBus.cpp \
    Bus.cpp \
    ClientSchemaWidget.cpp \
    SchemaManager.cpp \
    ClientSchemaView.cpp \
    SchemaItemLoopback.cpp \
    TuningController.cpp \
    AppSignalController.cpp \
    TuningSchema.cpp \
    SchemaItemImage.cpp \
    SchemaItemImageValue.cpp \
	ImageItem.cpp

DISTFILES +=


