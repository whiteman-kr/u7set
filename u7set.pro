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

AppDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib
DiagDataService.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib
ArchivingService.depends = Protobuf OnlineLib ServiceLib UtilsLib
ServiceControlManager.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib
ConfigurationService.depends = Protobuf OnlineLib ServiceLib UtilsLib
BaseService.depends = OnlineLib ServiceLib UtilsLib Protobuf
TuningService.depends = Protobuf OnlineLib ServiceLib UtilsLib

Monitor.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30
TuningClient.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30 HardwareLib

SimulatorConsole.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30
BuilderConsole.depends = Builder Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 DbLib

u7.depends = Builder Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30 HardwareLib TrendView QScintilla DbLib

SimulatorTests.depends = Protobuf HardwareLib Simulator UtilsLib OnlineLib VFrame30
u7databaseTests.depends = Protobuf UtilsLib HardwareLib DbLib OnlineLib

Metrology.depends = Protobuf OnlineLib UtilsLib TrendView VFrame30 HardwareLib DbLib
mconf.depends = Protobuf UtilsLib

PacketSource.depends = Protobuf ServiceLib OnlineLib UtilsLib HardwareLib
PacketViewer.depends = Protobuf OnlineLib UtilsLib HardwareLib

UalTestser.depends = Protobuf OnlineLib ServiceLib UtilsLib HardwareLib

