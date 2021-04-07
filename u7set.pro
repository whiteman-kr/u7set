TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += Protobuf \
	qtpropertybrowser \
	QScintilla \
	TrendView \
	VFrame30 \
	Simulator \
	SimulatorConsole \
	Builder \
	BuilderConsole \
	u7 \
	Metrology \
	qtservice \
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
	./Tools/CommView \
	./Tools/PacketSource \
	./Test/UalTester \
	./Test/SimulatorTests \
	./Test/u7databaseTests

