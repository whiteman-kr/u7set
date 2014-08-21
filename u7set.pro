TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    BaseService
SUBDIRS += VFrame30
SUBDIRS += u7

