TEMPLATE = subdirs

CONFIG += ordered

win32:	SUBDIRS += Protobuf \
	AppDataService \
	ConfigurationService \
	TuningService



