TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += ./Tools/PacketSource \
	   ./PacketSource



