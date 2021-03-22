TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Protobuf \
    qtservice \
	qtpropertybrowser \
	QScintilla \
	TrendView \
	VFrame30 \
	BaseService \
	ServiceControlManager \
	Metrology \
	Simulator \
	SimulatorConsole \
	Builder \
	BuilderConsole \
	u7 \
	ConfigurationService \
	ArchivingService \
	Monitor \
	PacketViewer \
	TuningService \
	AppDataService \
	DiagDataService \
	TuningClient \
	mconf \
	./Tools/CommView \
	./Tools/PacketSource \
	./Test/UalTester \
	./Test/SimulatorTests \
	./Test/u7databaseTests
