TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
    QScintilla \
    TrendView \
	VFrame30 \
	ServiceControlManager \
	Metrology \
    Simulator \
    SimulatorConsole \
    Builder \
    BuilderConsole \
    u7 \
    BaseService \
    ConfigurationService \
    ArchivingService \
    Monitor \
    PacketViewer \
    TuningService \
    AppDataService \
    DiagDataService \
    TuningClient \
    TuningIPEN \
    mconf \
	./Tools/CommView \
    ./Tools/PacketSource \
	./Test/SimulatorTests \
	./Test/u7databaseTests

