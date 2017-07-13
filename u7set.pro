TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtsingleapplication \
    qtpropertybrowser \
    GetGitProjectVersion \
    BaseService \
    ServiceControlManager \
    Metrology \
    ConfigurationService \
    ArchivingService \
	QScintilla \	
	VFrame30 \
	u7 \
    Monitor \
    PacketViewer \
    TuningService \
    AppDataService \
    DiagDataService \
    mconf \
    TuningClient \
    TuningIPEN \
    TrendView \
    Trends
