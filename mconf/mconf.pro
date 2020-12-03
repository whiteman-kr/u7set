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

# c++17 support
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
    ../lib/Ui/DialogAbout.h \
	Stable.h \
	ftdi/ftd2xx.h \
        ../lib/DbStruct.h \
    ../lib/DeviceObject.h \
    ../lib/OutputLog.h \
    ../Proto/serialization.pb.h \
    ../lib/ProtoSerialization.h \
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
    ../lib/Ui/DialogAbout.cpp \
	main.cpp \
	../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/OutputLog.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/ProtoSerialization.cpp \
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

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS


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
    Images/Logo.png \
    Images/MConf.ico

