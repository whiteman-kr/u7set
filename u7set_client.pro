TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtpropertybrowser \
 	GetGitProjectVersion \
	VFrame30 \
	u7 \
	Monitor \
	mconf


