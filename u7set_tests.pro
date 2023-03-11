TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Protobuf \
    TrendView \
	AppSignalLib \
	UtilsLib \
	OnlineLib \
	ServiceLib \
	ClientLib \
	HardwareLib \
	CommonLib \
	VFrame30 \
	DbLib \
	Simulator \
	Builder \
	ClientTests \
	SimulatorTests \
	MetrologyTests \
	u7databaseTests

ClientTests.subdir = ./Test/ClientTests
SimulatorTests.subdir = ./Test/SimulatorTests
MetrologyTests.subdir = ./Test/MetrologyTests
u7databaseTests.subdir = ./Test/u7databaseTests



