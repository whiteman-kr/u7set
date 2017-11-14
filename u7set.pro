TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
    GetGitProjectVersion \
    BaseService \
    ServiceControlManager \
    Metrology \
    ConfigurationService \
    ArchivingService \
	QScintilla \	
    TrendView \
	VFrame30 \
	u7 \
    Monitor \
    PacketViewer \
    TuningService \
    AppDataService \
    DiagDataService \
    TuningClient \
    TuningIPEN \
    mconf \
    LmModel \
    LmModelConsole

