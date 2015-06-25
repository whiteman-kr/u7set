#-------------------------------------------------
#
# Project created by QtCreator 2012-10-02T18:35:47
#
#-------------------------------------------------
TARGET = VFrame30

TEMPLATE = lib
QT += widgets qml script

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


# Force prebuild version control info
#
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
        contains(QMAKE_TARGET.arch, x86_64){
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/VFrame30.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/VFrame30.pro
        }
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/VFrame30.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


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
    Fbl.h \
    DrawParam.h \
    Print.h \
    VFrame30Library.h \
    HorzVertLinks.h \
	../include/TypesAndEnums.h \
    FrameHistoryItem.h \
	Configuration.h \
	DebugInstCounter.h \
    VFrame30.h \
    ../include/StreamedData.h \
    ../include/ProtoSerialization.h \
    ../include/CUtils.h \
    SchemeLayer.h \
    Scheme.h \
    WorkflowScheme.h \
    LogicScheme.h \
    DiagScheme.h \
    WiringScheme.h \
    SchemeView.h \
    version.h \
    ../include/DbStruct.h \
    BaseSchemeWidget.h \
    SchemeItemConst.h \
    SchemeItem.h \
    SchemeItemLine.h \
    SchemeItemRect.h \
    SchemeItemPath.h \
    SchemeItemSignal.h \
    SchemeItemAfb.h \
    SchemeItemLink.h

SOURCES += \
    Settings.cpp \
    PosRectImpl.cpp \
    PosLineImpl.cpp \
    PosConnectionImpl.cpp \
    FontParam.cpp \
    FblItemRect.cpp \
    FblItemLine.cpp \
    FblItem.cpp \
    Fbl.cpp \
    DrawParam.cpp \
    Print.cpp \
    Stable.cpp \
    VFrame30Library.cpp \
    HorzVertLinks.cpp \
    FrameHistoryItem.cpp \
	Configuration.cpp \
    ../lib/StreamedData.cpp \
    ../lib/ProtoSerialization.cpp \
    SchemeLayer.cpp \
    Scheme.cpp \
    WorkflowScheme.cpp \
    LogicScheme.cpp \
    DiagScheme.cpp \
    WiringScheme.cpp \
    SchemeView.cpp \
    ../lib/DbStruct.cpp \
    BaseSchemeWidget.cpp \
    SchemeItemConst.cpp \
    SchemeItem.cpp \
    SchemeItemLine.cpp \
    SchemeItemRect.cpp \
    SchemeItemPath.cpp \
    SchemeItemSignal.cpp \
    SchemeItemAfb.cpp \
    SchemeItemLink.cpp

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

