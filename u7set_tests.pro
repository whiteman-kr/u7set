TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Protobuf \
    TrendView \
	AppSignalLib \
	UtilsLib \
	OnlineLib \
	ServiceLib \
	HardwareLib \
	CommonLib \	
	ClientLib \
	ReportLib\
	VFrame30 \
	DbLib \
	Simulator \
	SimulatorConsole \
	Builder \
	BuilderConsole \
	ConfigurationService \
	ArchivingService \
	AppDataService \
	ClientTests \
	TuningService \
	SimulatorTests \
	MetrologyTests \
	u7databaseTests

ClientTests.subdir = ./Test/ClientTests
SimulatorTests.subdir = ./Test/SimulatorTests
MetrologyTests.subdir = ./Test/MetrologyTests
u7databaseTests.subdir = ./Test/u7databaseTests



