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

CONFIG += precompile_header

DEFINES += QT_DLL QT_WIDGETS_LIB QT_NETWORK_LIB QT_SQL_LIB QT_XML_LIB

win32:LIBS += advapi32.lib

HEADERS += crc.h \
	settings.h \
	Stable.h \
	configurator.h \
	applicationtabpage.h \
	diagtabpage.h \
	moduleconfigurator.h \
	settingsform.h \
	ftdi/ftd2xx.h \
	../include/DbStruct.h \
	../include/ConfigData.h \
    ../include/DeviceObject.h \
    ../include/OutputLog.h \
    ../Proto/serialization.pb.h \
    ../include/ModuleConfiguration.h \
    ../include/ProtoSerialization.h

SOURCES += applicationtabpage.cpp \
	configurator.cpp \
	crc.cpp \
	diagtabpage.cpp \
	main.cpp \
	moduleconfigurator.cpp \
	settings.cpp \
	settingsform.cpp \
	Stable.cpp \
	../lib/DbStruct.cpp \
	../lib/ConfigData.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/OutputLog.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/ModuleConfiguration.cpp \
    ../lib/ProtoSerialization.cpp

FORMS += moduleconfigurator.ui \
	diagtabpage.ui \
	applicationtabpage.ui

RESOURCES +=	moduleconfigurator.qrc

PRECOMPILED_HEADER = stable.h

OTHER_FILES +=

win32: LIBS += -L$$PWD/ftdi/ -lftd2xx

INCLUDEPATH += $$PWD/ftdi
DEPENDPATH += $$PWD/ftdi

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

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
