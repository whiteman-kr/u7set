TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtpropertybrowser \
 	GetGitProjectVersion \
	VFrame30 \
	TuningClient


