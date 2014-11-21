TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtsingleapplication \
    qtpropertybrowser \
    BaseService \
    ServiceControlManager \
    UdpClient \
    UdpServer \
    Metrology \
    ConfigurationService \
    ArchivingService \
    DataAquisitionService
SUBDIRS += VFrame30
SUBDIRS += u7

