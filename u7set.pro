TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Protobuf \
	qtpropertybrowser \
	QScintilla \
	TrendView \
	VFrame30 \
	OnlineLib \
	ServiceLib \
	Simulator \
	SimulatorConsole \
	Builder \
	BuilderConsole \
	u7 \
	Metrology \
	BaseService \
	ServiceControlManager \
	ConfigurationService \
	ArchivingService \
	Monitor \
	PacketViewer \
	TuningService \
	AppDataService \
	DiagDataService \
	TuningClient \
	mconf \
	./Tools/PacketSource \
	./Test/UalTester \
	./Test/SimulatorTests \
	./Test/u7databaseTests

AppDataService.depends = OnlineLib ServiceLib
DiagDataService.depends = OnlineLib ServiceLib
ArchivingService.depends = OnlineLib ServiceLib
ConfigurationService.depends = OnlineLib ServiceLib
BaseService.depends = OnlineLib ServiceLib
TuningService.depends = OnlineLib ServiceLib
