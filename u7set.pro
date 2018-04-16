TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
    GetGitProjectVersion \
    ServiceControlManager \
    Metrology \
    TrendView \
	VFrame30 \
    Lua \
    Simulator \
    SimulatorConsole \
    QScintilla \
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
    lib/IAppSignalManager.h

