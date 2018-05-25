TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
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
