TEMPLATE = subdirs

# Raspberry PI services and clients
#

CONFIG += ordered

SUBDIRS += Protobuf \
    TrendView \
    AppSignalLib \
	UtilsLib \
	OnlineLib \
	ServiceLib \
	HardwareLib \
	CommonLib \
	VFrame30 \
	Simulator \
	SimulatorConsole \
	ServiceControlManager \
	ConfigurationService \
	ArchivingService \
	Monitor \
	TuningService \
	AppDataService \	
	TuningClient \	
	SimulatorTests

SimulatorTests.subdir = ./Test/SimulatorTests
