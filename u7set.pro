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
	DataAquisitionService \
	VFrame30 \
	u7 \
    Monitor \
    PacketViewer \
    TcpClient

