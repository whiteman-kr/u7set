TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtsingleapplication \
    BaseService \
    ServiceControlManager \
    UdpClient \
    UdpServer
SUBDIRS += VFrame30
SUBDIRS += u7

