TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtsingleapplication \
    BaseService \
    ServiceControlManager \
    UdpClient \
    UdpServer \
    Metrology
SUBDIRS += VFrame30
SUBDIRS += u7

