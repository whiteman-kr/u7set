TEMPLATE = subdirs

CONFIG += ordered

win32:SUBDIRS += Protobuf
SUBDIRS += qtservice \
    qtpropertybrowser \
    QScintilla \
    ServiceControlManager \
    Metrology \
    TrendView \
	VFrame30 \
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
	./Test/u7databaseTests

