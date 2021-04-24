TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS += Protobuf \
        qtpropertybrowser \
        QScintilla \
        TrendView \
        VFrame30 \
        UtilsLib \
        OnlineLib \
        ServiceLib \
        HardwareLib \
		CommonLib \
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
        PacketSource \
        PacketViewer \
        UalTester \
        SimulatorTests \
        u7databaseTests

PacketSource.subdir = ./Tools/PacketSource
PacketViewer.subdir = ./Tools/PacketViewer
UalTester.subdir = ./Test/UalTester
SimulatorTests.subdir = ./Test/SimulatorTests
u7databaseTests.subdir = ./Test/u7databaseTests

AppDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib CommonLib
DiagDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib CommonLib
ArchivingService.depends = Protobuf OnlineLib ServiceLib UtilsLib CommonLib
ServiceControlManager.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib CommonLib
ConfigurationService.depends = Protobuf OnlineLib ServiceLib UtilsLib CommonLib
BaseService.depends = OnlineLib ServiceLib UtilsLib Protobuf CommonLib
TuningService.depends = Protobuf OnlineLib ServiceLib UtilsLib CommonLib

Monitor.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30 CommonLib
TuningClient.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30 HardwareLib CommonLib

SimulatorConsole.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 CommonLib
BuilderConsole.depends = Builder Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 DbLib CommonLib

u7.depends = Builder Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 HardwareLib TrendView QScintilla DbLib CommonLib

SimulatorTests.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 CommonLib
u7databaseTests.depends = Protobuf UtilsLib HardwareLib DbLib OnlineLib CommonLib

Metrology.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30 HardwareLib DbLib CommonLib
mconf.depends = Protobuf UtilsLib CommonLib

PacketSource.depends = Protobuf ServiceLib OnlineLib UtilsLib HardwareLib CommonLib
PacketViewer.depends = Protobuf OnlineLib UtilsLib HardwareLib CommonLib

UalTestser.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib CommonLib

