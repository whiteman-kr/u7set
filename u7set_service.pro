TEMPLATE = subdirs

CONFIG += ordered

win32:	SUBDIRS +=  Protobuf \
		    qtservice \
                    BaseService \
                    ConfigurationService \
                    AppDataService \
                    TuningService



