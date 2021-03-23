##-------------------------------------------------
##
## Project created by QtCreator 2013-05-15T14:47:21
##
##-------------------------------------------------

#QT       += core gui

#TARGET = mconf
#TEMPLATE = app


#SOURCES += main.cpp\
#        mainwindow.cpp

#HEADERS  += mainwindow.h

#FORMS    += mainwindow.ui

QT += core sql network xml widgets gui serialport qml

TARGET = mconf
TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

#Application icon
win32:RC_ICONS += Images/MConf.ico

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

DEFINES += QT_DLL QT_WIDGETS_LIB QT_NETWORK_LIB QT_SQL_LIB QT_XML_LIB

#win32:LIBS += advapi32.lib

HEADERS += \
    ../lib/ScriptDeviceObject.h \
    ../lib/Ui/DialogAbout.h \
	Stable.h \
	ftdi/ftd2xx.h \
        ../lib/DbStruct.h \
    ../lib/DeviceObject.h \
    ../lib/OutputLog.h \
    Settings.h \
    ApplicationTabPage.h \
    DiagTabPage.h \
    ModuleConfigurator.h \
    SettingsForm.h \
        ../lib/Crc.h \
        ../lib/CUtils.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/Configurator.h \
    ../lib/ModuleFirmware.h

SOURCES += \
    ../lib/ScriptDeviceObject.cpp \
    ../lib/Ui/DialogAbout.cpp \
	main.cpp \
	../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/OutputLog.cpp \
    Settings.cpp \
    ApplicationTabPage.cpp \
    DiagTabPage.cpp \
    ModuleConfigurator.cpp \
    SettingsForm.cpp \
    ../lib/Crc.cpp \
    ../lib/Types.cpp \
    ../lib/Configurator.cpp \
    ../lib/ModuleFirmware.cpp

FORMS += moduleconfigurator.ui \
	diagtabpage.ui

RESOURCES +=	moduleconfigurator.qrc

CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

OTHER_FILES +=

#win32: LIBS += -L$$PWD/ftdi -lftd2xx
win32: LIBS += -L$$PWD/ftdi64 -lftd2xx

INCLUDEPATH += $$PWD/ftdi
DEPENDPATH += $$PWD/ftdi

#Optimization flags
#
win32 {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    Images/Logo.png \
    Images/MConf.ico

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
