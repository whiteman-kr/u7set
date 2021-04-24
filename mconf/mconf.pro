QT += core sql network xml widgets gui serialport qml

TARGET = mconf
TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# Optimization flags
#
win32 {
        CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
        CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
        CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
        CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

# Application icon
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
PRECOMPILED_HEADER = stable.h

HEADERS += \
    ../lib/Ui/DialogAbout.h \
	../lib/OutputLog.h \
	../lib/CUtils.h \
	../CommonLib/PropertyObject.h \
	../lib/Configurator.h \
	Stable.h \
	ftdi/ftd2xx.h \
    Settings.h \
    ApplicationTabPage.h \
    DiagTabPage.h \
    ModuleConfigurator.h \
    SettingsForm.h \

SOURCES += \
    ../lib/Ui/DialogAbout.cpp \
	../lib/OutputLog.cpp \
	../lib/Configurator.cpp \
	main.cpp \
    Settings.cpp \
    ApplicationTabPage.cpp \
    DiagTabPage.cpp \
    ModuleConfigurator.cpp \
    SettingsForm.cpp \

FORMS += moduleconfigurator.ui \
	diagtabpage.ui

RESOURCES +=	moduleconfigurator.qrc

DISTFILES += \
    Images/Logo.png \
    Images/MConf.ico


# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

#win32:LIBS += advapi32.lib

#win32: LIBS += -L$$PWD/ftdi -lftd2xx
win32: LIBS += -L$$PWD/ftdi64 -lftd2xx

INCLUDEPATH += $$PWD/ftdi
DEPENDPATH += $$PWD/ftdi

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
    CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# HardwareLib
#
LIBS += -lHardwareLib

# UtilsLib
#
LIBS += -lUtilsLib

# CommonLib
#
LIBS += -lCommonLib

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf
