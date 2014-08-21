TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    BaseService \
    ServiceControlManager
SUBDIRS += VFrame30
SUBDIRS += u7

