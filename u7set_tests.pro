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
	VFrame30 \
	DbLib \
	Simulator \
	Builder \
	SimulatorTests \
	MetrologyTests \
	u7databaseTests

SimulatorTests.subdir = ./Test/SimulatorTests
MetrologyTests.subdir = ./Test/MetrologyTests
u7databaseTests.subdir = ./Test/u7databaseTests



