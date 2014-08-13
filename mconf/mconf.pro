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

QT += core sql network xml widgets gui

TARGET = mconf
TEMPLATE = app

CONFIG += precompile_header

DEFINES += QT_DLL QT_WIDGETS_LIB QT_NETWORK_LIB QT_SQL_LIB QT_XML_LIB

win32:LIBS += advapi32.lib

HEADERS += crc.h \
	log.h \
	settings.h \
	stable.h \
	configurator.h \
	applicationtabpage.h \
	diagtabpage.h \
	moduleconfigurator.h \
	settingsform.h \
	ftdi/ftd2xx.h \
    ../include/dbstruct.h \
    ../include/dbstore.h \
    ../include/configdata.h \
    ../include/DeviceObject.h

SOURCES += applicationtabpage.cpp \
	configurator.cpp \
	crc.cpp \
	diagtabpage.cpp \
	log.cpp \
	main.cpp \
	moduleconfigurator.cpp \
	settings.cpp \
	settingsform.cpp \
	stable.cpp \
    ../lib/dbstruct.cpp \
    ../lib/dbstore.cpp \
    ../lib/configdata.cpp \
    ../lib/DeviceObject.cpp

FORMS += moduleconfigurator.ui \
	diagtabpage.ui \
	applicationtabpage.ui

RESOURCES +=	moduleconfigurator.qrc

PRECOMPILED_HEADER = stable.h

OTHER_FILES +=

win32: LIBS += -L$$PWD/ftdi/ -lftd2xx

INCLUDEPATH += $$PWD/ftdi
DEPENDPATH += $$PWD/ftdi
