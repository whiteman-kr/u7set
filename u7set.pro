TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
    Lua \
    QScintilla \
    GetGitProjectVersion \
    ServiceControlManager \
    Metrology \
    TrendView \
	VFrame30 \
    Simulator \
    SimulatorConsole \
	u7 \
    BaseService \
    ConfigurationService \
    ArchivingService \
    Monitor \
    PacketViewer \
    TuningService \
    AppDataService \
    DiagDataService \
    TuningClient \
    TuningIPEN \
    mconf

HEADERS += \
    lib/IAppSignalManager.h \
    lib/CommonTypes.h \
    lib/Times.h

SOURCES += \
    lib/Times.cpp
