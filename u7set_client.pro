TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtpropertybrowser \
 	GetGitProjectVersion \
	TrendView \
	VFrame30 \
	Builder \
	BuilderConsole \
	u7 \
	Monitor \
	Simulator \
	SimulatorConsole \
	mconf \
	TuningClient



