TEMPLATE = subdirs

SUBDIRS += Protobuf \
        qtpropertybrowser \
        QScintilla \
        TrendView \
        VFrame30 \
		UtilsLib \
        OnlineLib \
        ServiceLib \
		HardwareLib \
		DbLib \
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
        TuningService \
        AppDataService \
        DiagDataService \
        TuningClient \
        mconf \
        ./Tools/PacketSource \
		./Tools/PacketViewer \
        ./Test/UalTester \
        ./Test/SimulatorTests \
        ./Test/u7databaseTests

AppDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib
DiagDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib
ArchivingService.depends = Protobuf OnlineLib ServiceLib UtilsLib
ConfigurationService.depends = Protobuf OnlineLib ServiceLib UtilsLib
BaseService.depends = OnlineLib ServiceLib UtilsLib
TuningService.depends = Protobuf OnlineLib ServiceLib UtilsLib

Monitor.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30

SimulatorConsole.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30
BuilderConsole.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 DbLib

u7.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 HardwareLib TrendView QScintilla DbLib

SimulatorTests.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30
u7databaseTests.depends = Protobuf UtilsLib HardwareLib DbLib OnlineLib

