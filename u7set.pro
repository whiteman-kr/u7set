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

AppDataService.depends = OnlineLib ServiceLib UtilsLib
DiagDataService.depends = OnlineLib ServiceLib UtilsLib
ArchivingService.depends = OnlineLib ServiceLib UtilsLib
ConfigurationService.depends = OnlineLib ServiceLib UtilsLib
BaseService.depends = OnlineLib ServiceLib UtilsLib
TuningService.depends = OnlineLib ServiceLib UtilsLib
