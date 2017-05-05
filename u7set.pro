TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtsingleapplication \
    qtpropertybrowser \
    GetGitProjectVersion \
    BaseService \
    ServiceControlManager \
	UdpClient \
	UdpServer \
    Metrology \
    ConfigurationService \
    ArchivingService \
	QScintilla \	
	VFrame30 \
	u7 \
    Monitor \
    PacketViewer \
    TcpClient \
    TuningService \
    AppDataService \
    DiagDataService \
    mconf \
    TuningClient \
    TuningIPEN \
    TrendView \
    Trends
