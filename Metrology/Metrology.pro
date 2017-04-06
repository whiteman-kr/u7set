#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T12:56:12
#
#-------------------------------------------------

QT       += core gui widgets concurrent serialport network sql qml xml

TARGET = Metrology
TEMPLATE = app

include(../qtpropertybrowser/src/qtpropertybrowser.pri)

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


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

## Force prebuild version control info
##
#win32 {
#	contains(QMAKE_TARGET.arch, x86_64){
#	    system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
#	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#	    nmake))
#	    system(chdir $$PWD & \
#	    $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/Metrology.pro)
#	}
#	else{
#	    system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
#	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#	    nmake))
#	    system(chdir $$PWD & \
#	    $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/Metrology.pro)
#	}
#}
#unix {
#    system(cd $$PWD/../GetGitProjectVersion; \
#	qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
#	make; \
#	cd $$PWD; \
#	$$PWD/../bin_unix/GetGitProjectVersion $$PWD/Metrology.pro)
#}


SOURCES += \
    MainWindow.cpp \
    Measure.cpp \
    Calibrator.cpp \
    CalibratorBase.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    OptionsPointsDialog.cpp \
    main.cpp \
    MeasureThread.cpp \
    CalibratorManager.cpp \
    MeasureViewHeader.cpp \
    MeasureView.cpp \
    OptionsMvhDialog.cpp \
    Delegate.cpp \
    FolderPropertyManager.cpp \
    Database.cpp \
    Conversion.cpp \
    Calculator.cpp \
    ../lib/Crc.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/ModuleConfiguration.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Signal.cpp \
    ../lib/SocketIO.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SimpleThread.cpp \
    SignalSocket.cpp \
    ../lib/Tcp.cpp \
    SignalBase.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/AppSignalState.cpp \
    SignalList.cpp \
    FindMeasurePanel.cpp \
    SignalInfoPanel.cpp \
    Statistic.cpp \
    MeasurementBase.cpp \
    ExportData.cpp \
    FindData.cpp \
    TuningSocket.cpp \
    TuningSignalBase.cpp \
    TuningSignalList.cpp \
    ConfigSocket.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../u7/Builder/ModulesRawData.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
    ../lib/MetrologySignal.cpp \
    RackList.cpp \
    OutputSignalList.cpp \
    ObjectProperties.cpp \
    OutputSignalBase.cpp \
    RackBase.cpp



HEADERS  += \
    MainWindow.h \
    Measure.h \
    Calibrator.h \
    CalibratorBase.h \
    OptionsDialog.h \
    Options.h \
    OptionsPointsDialog.h \
    MeasureThread.h \
    CalibratorManager.h \
    MeasureViewHeader.h \
    MeasureView.h \
    OptionsMvhDialog.h \
    Delegate.h \
    FolderPropertyManager.h \
    Database.h \
    version.h \
    Conversion.h \
    Calculator.h \
    Stable.h \
    ObjectVector.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/Crc.h \
    ../lib/Factory.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ModuleConfiguration.h \
    ../lib/ProtoSerialization.h \
    ../lib/Types.h \
    ../lib/OrderedHash.h \
    ../lib/SocketIO.h \
    ../lib/PropertyObject.h \
    ../lib/XmlHelper.h \
    ../lib/HostAddressPort.h \
    ../lib/SimpleThread.h \
    SignalSocket.h \
    ../lib/Tcp.h \
    SignalBase.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/AppSignalState.h \
    SignalList.h \
    FindMeasurePanel.h \
    SignalInfoPanel.h \
    Statistic.h \
    MeasurementBase.h \
    ExportData.h \
    FindData.h \
    TuningSocket.h \
    TuningSignalBase.h \
    TuningSignalList.h \
    ConfigSocket.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/TcpFileTransfer.h \
    ../lib/ServiceSettings.h \
    ../lib/DeviceHelper.h \
    ../u7/Builder/ModulesRawData.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/OutputLog.h \
    ../lib/MetrologySignal.h \
    RackList.h \
    OutputSignalList.h \
    ObjectProperties.h \
    OutputSignalBase.h \
    RackBase.h


FORMS    +=

RESOURCES += \
    Resources.qrc

TRANSLATIONS = translations/Metrology_ru.ts \
		translations/Metrology_uk.ts

OTHER_FILES += \
    translations/Metrology_ru.ts \
    translations/Metrology_uk.ts


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG


# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
		LIBS += -L$$DESTDIR -lprotobuf

		INCLUDEPATH += ./../Protobuf
}
unix {
		LIBS += -lprotobuf
}

# Visual Leak Detector
#
win32{
	contains(QMAKE_TARGET.arch, x86_64){
		LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
		LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	}
	else{
		LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
		LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
	}

	INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
	INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto
