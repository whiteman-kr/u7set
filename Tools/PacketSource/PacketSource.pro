#-------------------------------------------------
#
# Project created by QtCreator 2018-03-27T13:22:36
#
#-------------------------------------------------

QT       += core gui widgets network sql qml xml

TARGET = PacketSource
TEMPLATE = app


# DESTDIR
#
win32 {
CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

# Force prebuild version control info
#
win32 {
contains(QMAKE_TARGET.arch, x86_64){
QMAKE_CLEAN += $$PWD/../../bin_Win64/GetGitProjectVersion.exe
system(IF NOT EXIST $$PWD/../../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../../GetGitProjectVersion & \
qmake \"OBJECTS_DIR = $$OUT_PWD/../../GetGitProjectVersion/release\" & \
nmake))
system(chdir $$PWD & \
$$PWD/../../bin_Win64/GetGitProjectVersion.exe $$PWD/PacketSource.pro)
}
else{
QMAKE_CLEAN += $$PWD/../../bin_Win32/GetGitProjectVersion.exe
system(IF NOT EXIST $$PWD/../../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../../GetGitProjectVersion & \
qmake \"OBJECTS_DIR = $$OUT_PWD/../../GetGitProjectVersion/release\" & \
nmake))
system(chdir $$PWD & \
$$PWD/../../bin_Win32/GetGitProjectVersion.exe $$PWD/PacketSource.pro)
}
}
unix {
QMAKE_CLEAN += $$PWD/../../bin_unix/GetGitProjectVersion
system(cd $$PWD/../../GetGitProjectVersion; \
qmake \"OBJECTS_DIR = $$OUT_PWD/../../GetGitProjectVersion/release\"; \
make; \
cd $$PWD; \
$$PWD/../../bin_unix/GetGitProjectVersion $$PWD/PacketSource.pro)
}

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    Options.cpp \
    SourceOptionDialog.cpp \
    SourceWorker.cpp \
    SourceBase.cpp \
../../lib/XmlHelper.cpp \
../../lib/SocketIO.cpp \
../../lib/HostAddressPort.cpp \
../../lib/PlainObjectHeap.cpp \
../../lib/SimpleThread.cpp \
../../lib/Crc.cpp \
../../lib/DataProtocols.cpp \
../../lib/WUtils.cpp \

HEADERS += \
        MainWindow.h \
    Options.h \
    SourceOptionDialog.h \
    SourceWorker.h \
    SourceBase.h \
../../lib/XmlHelper.h \
../../lib/SocketIO.h \
../../lib/HostAddressPort.h \
../../lib/SimpleThread.h \
../../lib/Crc.h \
../../lib/DataProtocols.h \
../../lib/WUtils.h \
    ../../u7/Builder/CfgFiles.h \

RESOURCES += \
    resources.qrc


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG

# Visual Leak Detector
#
#win32 {
#contains(QMAKE_TARGET.arch, x86_64) {
#LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
#LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
#} else {
#LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
#LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
#}

#INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
#INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
#}
