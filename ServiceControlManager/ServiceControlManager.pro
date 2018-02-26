#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network qml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scm
TEMPLATE = app

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


# Force prebuild version control info
#
win32 {
    contains(QMAKE_TARGET.arch, x86_64){
        QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
        system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake))
        system(chdir $$PWD & \
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/ServiceControlManager.pro)
    }
    else{
        QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
        system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake))
        system(chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/ServiceControlManager.pro)
    }
}
unix {
    QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
    system(cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/ServiceControlManager.pro)
}


SOURCES += MainWindow.cpp \
    ScanOptionsWidget.cpp \
    ServiceTableModel.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/DataSource.cpp \
    main.cpp \
    ../lib/JsonSerializable.cpp \
    BaseServiceStateWidget.cpp \
    ConfigurationServiceWidget.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    TcpAppDataClient.cpp \
    ../lib/Tcp.cpp \
    ../Proto/network.pb.cc \
    ../lib/Signal.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/AppSignal.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Types.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/AppDataSource.cpp \
    ../lib/CircularLogger.cpp \
    TcpConfigServiceClient.cpp \
    AppDataServiceWidget.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/Service.cpp \
    ../lib/CommandLineParser.cpp \
    ../lib/TuningValue.cpp \
    TuningServiceWidget.cpp \
    TcpTuningServiceClient.cpp \
    ../lib/Tuning/TuningSourceState.cpp

HEADERS  += MainWindow.h \
    ScanOptionsWidget.h \
    ServiceTableModel.h \
    ../lib/UdpSocket.h \
    ../lib/SocketIO.h \
    ../lib/DataSource.h \
    version.h \
    ../lib/JsonSerializable.h \
    BaseServiceStateWidget.h \
    ConfigurationServiceWidget.h \
    ../lib/SimpleThread.h \
    ../lib/XmlHelper.h \
    ../lib/Queue.h \
    ../lib/DataProtocols.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    TcpAppDataClient.h \
    ../lib/Tcp.h \
    ../Proto/network.pb.h \
    ../lib/Signal.h \
    ../lib/PropertyObject.h \
    ../Proto/serialization.pb.h \
    ../lib/AppSignal.h \
    ../lib/DeviceObject.h \
    ../lib/Types.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/HostAddressPort.h \
    ../lib/AppDataSource.h \
    ../lib/CircularLogger.h \
    TcpConfigServiceClient.h \
    ../lib/WidgetUtils.h \
    AppDataServiceWidget.h \
    ../lib/SoftwareInfo.h \
    ../lib/OrderedHash.h \
    ../lib/Service.h \
    ../lib/CommandLineParser.h \
    ../lib/TuningValue.h \
    TuningServiceWidget.h \
    TcpTuningServiceClient.h \
    ../lib/Tuning/TuningSourceState.h

include(../qtservice/src/qtservice.pri)

FORMS    +=

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
                ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc

gcc:QMAKE_CXXFLAGS += -std=c++11
win32:QMAKE_CXXFLAGS += /std:c++17

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto
