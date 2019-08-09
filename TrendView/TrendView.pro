#-------------------------------------------------
#
# Project created by QtCreator 2017-04-04T09:30:09
#
#-------------------------------------------------

QT       += core widgets svg gui printsupport

TARGET = TrendView
TEMPLATE = lib

CONFIG += staticlib

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet
win32:QMAKE_CXXFLAGS += /analyze

# Warning level
#
gcc:CONFIG += warn_on

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TrendWidget.cpp \
    TrendSignal.cpp \
    TrendSlider.cpp \
    TrendSettings.cpp \
    TrendMainWindow.cpp \
    Trend.cpp \
    DialogTrendSignalProperties.cpp \
    ../Proto/trends.pb.cc \
    TrendRuler.cpp \
    TrendParam.cpp \
    DialogTrendSignalPoints.cpp \
    DialogTrendSignalPoint.cpp

HEADERS += \
    Stable.h \
    TrendWidget.h \
    TrendSignal.h \
    ../lib/TimeStamp.h \
    TrendSlider.h \
    TrendSettings.h \
    TrendMainWindow.h \
    Trend.h \
    DialogTrendSignalProperties.h \
    ../Proto/trends.pb.h \
    TrendRuler.h \
    TrendParam.h \
    ../lib/CUtils.h \
    DialogTrendSignalPoints.h \
    DialogTrendSignalPoint.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    TrendView.qrc

FORMS += \
    TrendsMainWindow.ui \
    DialogTrendSignalProperties.ui \
    DialogTrendSignalPoints.ui \
    DialogTrendSignalPoint.ui


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
    ../Proto/trends.proto

