#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg serialport xml

TARGET = u7
TEMPLATE = app

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD


# DESTDIR
# If you see somewhere 'LNK1146: no argument specified with option '/LIBPATH:' then most likely you have not added this section to a project file
#
win32 {
    CONFIG(debug, debug|release): DESTDIR = ../bin/debug
    CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
    CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
    CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}
# /DESTDIR
#

CONFIG(debug, debug|release) {
    OBJECTS_DIR = debug
    MOC_DIR = debug/moc
    RCC_DIR = debug/rcc
    UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
    OBJECTS_DIR = release
    MOC_DIR = release/moc
    RCC_DIR = release/rcc
    UI_DIR = release/ui
}


# Force prebuild version control info
#
win32 {
    contains(QMAKE_TARGET.arch, x86_64){
        QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
        system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake))
        system(chdir $$PWD & \
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/u7.pro)
    }
    else{
        QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
        system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake))
        system(chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/u7.pro)
    }
}
unix {
    QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
    system(cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/u7.pro)
}


SOURCES +=\
    CentralWidget.cpp \
    CreateProjectDialog.cpp \
    CreateUserDialogDialog.cpp \
    DialogSettings.cpp \
    FilesTabPage.cpp \
    LoginDialog.cpp \
    Main.cpp \
    MainTabPage.cpp \
    MainWindow.cpp \
    PasswordService.cpp \
    Settings.cpp \
    UserManagementDialog.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbController.cpp \
    ../lib/DbWorker.cpp \
    ../lib/DbProgressDialog.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Signal.cpp \
    EquipmentTabPage.cpp \
    CheckInDialog.cpp \
    ProjectsTabPage.cpp \
    SignalsTabPage.cpp \
    SignalPropertiesDialog.cpp \
    EditEngine/EditEngine.cpp \
    EditEngine/EditEngineAddItem.cpp \
    EditEngine/EditEngineDeleteItem.cpp \
    EditEngine/EditEngineMoveItem.cpp \
    EditEngine/EditEngineSetPoints.cpp \
    EditEngine/EditEngineSetProperty.cpp \
    ../lib/ModuleConfiguration.cpp \
    BuildTabPage.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbProgress.cpp \
    ../lib/Crc.cpp \
    DialogFileEditor.cpp \
    Builder/Builder.cpp \
    Builder/BuildResultWriter.cpp \
    Builder/ConfigurationBuilder.cpp \
    Builder/ApplicationLogicCode.cpp \
    Builder/ApplicationLogicCompiler.cpp \
    DialogSubsystemListEditor.cpp \
    Subsystem.cpp \
    EquipmentVcsDialog.cpp \
    ../lib/DataSource.cpp \
    ../lib/SocketIO.cpp \
    Builder/FbParamCalculation.cpp \
    ../lib/PropertyEditorOld.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/Types.cpp \
    Builder/Parser.cpp \
    Connection.cpp \
    ../lib/PropertyEditorDialog.cpp \
    Builder/SoftwareCfgGenerator.cpp \
    ../lib/BuildInfo.cpp \
    Builder/TuningBuilder.cpp \
    Builder/IssueLogger.cpp \
    Builder/OptoModule.cpp \
    GlobalMessanger.cpp \
    Builder/LmMemoryMap.cpp \
    Builder/ModuleLogicCompiler.cpp \
    EditSchemaWidget.cpp \
    SchemaPropertiesDialog.cpp \
    SchemaItemPropertiesDialog.cpp \
    SchemaLayersDialog.cpp \
    SchemaTabPage.cpp \
    CreateSchemaDialog.cpp \
    EditEngine/EditEngineSetSchemaProperty.cpp \
    EditEngine/EditEngineSetOrder.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/XmlHelper.cpp \
    Builder/MonitorCfgGenerator.cpp \
    ../lib/ServiceSettings.cpp \
    Builder/TuningServiceCfgGenerator.cpp \
    Builder/AppDataServiceCfgGenerator.cpp \
    Builder/DiagDataServiceCfgGenerator.cpp \
    ../lib/Queue.cpp \
    ../lib/ProtobufHelper.cpp \
    UploadTabPage.cpp \
    ../TuningService/TuningSource.cpp \
    DialogChoosePreset.cpp \
    ../lib/WUtils.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../lib/DataProtocols.cpp \
    Builder/ModuleFirmwareWriter.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Configurator.cpp \
    DialogSettingsConfigurator.cpp \
    Builder/ArchivingServiceCfgGenerator.cpp \
    Builder/TuningClientCfgGenerator.cpp \
    Forms/ChooseUfbDialog.cpp \
    Builder/ModulesRawData.cpp \
    Builder/BdfFile.cpp \
    SchemaListModel.cpp \
    Forms/SelectChangesetDialog.cpp \
    Forms/FileHistoryDialog.cpp \
    Forms/ChangesetDetailsDialog.cpp \
    Forms/CompareDialog.cpp \
    Forms/ComparePropertyObjectDialog.cpp \
    ../TuningIPEN/TuningIPENDataStorage.cpp \
    DialogConnections.cpp \
    Builder/MetrologyCfgGenerator.cpp \
    ../lib/MetrologySignal.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/Tuning/TuningFilterEditor.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/Tuning/TuningModel.cpp \
    Builder/ComparatorStorage.cpp \
    Builder/RawDataDescription.cpp \
    ../lib/Tuning/TuningSignalStorage.cpp \
    ../lib/AppSignal.cpp \
    ../lib/CsvFile.cpp \
    DialogBusEditor.cpp \
    BusStorage.cpp \
    Forms/DialogUpdateFromPreset.cpp \
    Forms/ChooseAfbDialog.cpp \
    Builder/SignalSet.cpp \
    Builder/Busses.cpp \
    IdePropertyEditor.cpp \
    Builder/UalItems.cpp \
    EditEngine/EditEngineSetObject.cpp \
    ../lib/Address16.cpp \
    EditConnectionLine.cpp \
    EditEngine/EditEngineBatch.cpp \
    CreateSignalDialog.cpp \
    LogicModuleSet.cpp \
    ../lib/LmDescription.cpp

HEADERS  += \
    CentralWidget.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogSettings.h \
    FilesTabPage.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Settings.h \
    Stable.h \
    UserManagementDialog.h \
    ../lib/DbStruct.h \
    ../lib/DeviceObject.h \
    ../lib/DbController.h \
    ../lib/DbWorker.h \
    ../lib/DbProgressDialog.h \
    ../lib/ProtoSerialization.h \
    ../lib/Factory.h \
    ../lib/CUtils.h \
    ../lib/Signal.h \
    ../lib/OrderedHash.h \
    EquipmentTabPage.h \
    CheckInDialog.h \
    ProjectsTabPage.h \
    SignalsTabPage.h \
    SignalPropertiesDialog.h \
    EditEngine/EditEngine.h \
    EditEngine/EditEngineAddItem.h \
    EditEngine/EditEngineDeleteItem.h \
    EditEngine/EditEngineMoveItem.h \
    EditEngine/EditEngineSetPoints.h \
    EditEngine/EditEngineSetProperty.h \
    ../lib/ModuleConfiguration.h \
    BuildTabPage.h \
    ../lib/OutputLog.h \
    ../lib/DbProgress.h \
    version.h \
    ../lib/Crc.h \
    DialogFileEditor.h \
    Builder/Builder.h \
    Builder/BuildResultWriter.h \
    Builder/ConfigurationBuilder.h \
    Builder/ApplicationLogicCode.h \
    Builder/ApplicationLogicCompiler.h \
    DialogSubsystemListEditor.h \
    ../lib/Types.h \
    Subsystem.h \
    Forms/ChooseAfbDialog.h \
    EquipmentVcsDialog.h \
    ../lib/DataSource.h \
    ../lib/SocketIO.h \
    ../lib/PropertyObject.h \
    ../lib/PropertyEditorOld.h \
    ../lib/PropertyEditor.h \
    Builder/Parser.h \
    Connection.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/DebugInstCounter.h \
    Builder/SoftwareCfgGenerator.h \
    ../lib/BuildInfo.h \
    Builder/TuningBuilder.h \
    Builder/IssueLogger.h \
    Builder/OptoModule.h \
    GlobalMessanger.h \
    Builder/LmMemoryMap.h \
    ../lib/Address16.h \
    Builder/ModuleLogicCompiler.h \
    ../lib/WUtils.h \
    EditSchemaWidget.h \
    SchemaPropertiesDialog.h \
    SchemaItemPropertiesDialog.h \
    SchemaTabPage.h \
    SchemaLayersDialog.h \
    CreateSchemaDialog.h \
    EditEngine/EditEngineSetSchemaProperty.h \
    EditEngine/EditEngineSetOrder.h \
    ../lib/DeviceHelper.h \
    ../lib/XmlHelper.h \
    Builder/MonitorCfgGenerator.h \
    ../lib/ServiceSettings.h \
    Builder/TuningServiceCfgGenerator.h \
    Builder/AppDataServiceCfgGenerator.h \
    Builder/DiagDataServiceCfgGenerator.h \
    ../lib/Queue.h \
    ../lib/ProtobufHelper.h \
    UploadTabPage.h \
    ../TuningService/TuningSource.h \
    DialogChoosePreset.h \
    ../TuningService/TuningDataStorage.h \
    ../lib/DataProtocols.h \
    Builder/ModuleFirmwareWriter.h \
    ../lib/HostAddressPort.h \
    ../lib/SignalProperties.h \
    ../lib/Configurator.h \
    DialogSettingsConfigurator.h \
    Builder/ArchivingServiceCfgGenerator.h \
    Builder/TuningClientCfgGenerator.h \
    Forms/ChooseUfbDialog.h \
    ../lib/LmLimits.h \
    Builder/ModulesRawData.h \
    Builder/BdfFile.h \
    SchemaListModel.h \
    Forms/SelectChangesetDialog.h \
    Forms/FileHistoryDialog.h \
    Forms/ChangesetDetailsDialog.h \
    Forms/CompareDialog.h \
    Forms/ComparePropertyObjectDialog.h \
    ../lib/diff_match_patch.h \
    ../TuningIPEN/TuningIPENDataStorage.h \
    DialogConnections.h \
    Builder/MetrologyCfgGenerator.h \
    ../lib/MetrologySignal.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/Tuning/TuningFilterEditor.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/TuningModel.h \
    Builder/ComparatorStorage.h \
    Builder/RawDataDescription.h \
    ../lib/Tuning/TuningSignalStorage.h \
    ../lib/AppSignal.h \
    ../lib/CsvFile.h \
    ../lib/WidgetUtils.h \
    DialogBusEditor.h \
    DbObjectStorage.h \
    BusStorage.h \
    Forms/DialogUpdateFromPreset.h \
    Builder/SignalSet.h \
    Builder/Busses.h \
    IdePropertyEditor.h \
    Builder/UalItems.h \
    EditEngine/EditEngineSetObject.h \
    EditConnectionLine.h \
    EditEngine/EditEngineBatch.h \
    CreateSignalDialog.h \
    LogicModuleSet.h \
    ../lib/LmDescription.h

FORMS    += \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    LoginDialog.ui \
    UserManagementDialog.ui \
    CheckInDialog.ui \
    DialogSubsystemListEditor.ui \
    Forms/ChooseAfbDialog.ui \
    EquipmentVcsDialog.ui \
    CreateSchemaDialog.ui \
    SchemaLayersDialog.ui \
    SchemaPropertiesDialog.ui \
    SchemaItemPropertiesDialog.ui \
    DialogChoosePreset.ui \
    DialogSettingsConfigurator.ui \
    Forms/ChooseUfbDialog.ui \
    Forms/SelectChangesetDialog.ui \
    Forms/FileHistoryDialog.ui \
    Forms/ChangesetDetailsDialog.ui \
    Forms/CompareDialog.ui \
    Forms/ComparePropertyObjectDialog.ui \
    DialogTuningClients.ui

RESOURCES += \
    Resources.qrc \
    ../DatabaseUpgrade/DatabaseUpgrade.qrc

OTHER_FILES += \
    ../Proto/proto_compile.bat \
    ../Proto/serialization.proto \
    Tools/afbschema.xsd \
    ../Proto/proto_compile.sh \
    DatabaseUpgrade/Upgrade0039.txt

DISTFILES += \
    LogicModuleConfiguration.js \
    Afbl/_convert_all.bat \
    Afbl/bcomp_great_v1.afb \
    Afbl/bcomp_less_v1.afb \
    Afbl/bcomp_ne_v1.afb \
    Afbl/delay_on_v1.afb \
    Afbl/not_v1.afb \
    Afbl/maj_v1.afb \
    Afbl/or_v1.afb \
    Afbl/xor_v1.afb \
    Afbl/and_v1.afb \
    Afbl/bcomp_eq_v1.afb \
    Afbl/_afbl_all.sql \
    Afbl/tct_filter_v1.afb \
    Afbl/file2pgsql.exe \
    Afbl/tct_off_v1.afb \
    Afbl/tct_on_v1.afb \
    Afbl/tct_vibr_v1.afb \
    Ufbl/UFB_A3_LANDSCAPE.templ_ufb \
    Ufbl/UFB_A4_LANDSCAPE.templ_ufb \
    Ufbl/file2pgsql.exe \
    Ufbl/_convert_all.bat \
    LogicModuleDescription/LogicModule0000.xml \
    ../Proto/network.proto \
    LogicModuleDescription/LM1_SF00.xml \
    LogicModuleDescription/LM1_SR01.xml

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

win32 {
    #CONFIG(debug, debug|release): DEFINES += _CRTDBG_MAP_ALLOC
    #CONFIG(debug, debug|release): DEFINES += "DBG_NEW=new(_NORMAL_BLOCK,__FILE__,__LINE__)"
    #CONFIG(debug, debug|release): DEFINES += "new=DBG_NEW"
}



CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# c++14 support
#
win32:QMAKE_CXXFLAGS += -std:c++14
unix:QMAKE_CXXFLAGS += -std=c++14

#c++14 support for Windows
#
unix:QMAKE_CXXFLAGS += -std=c++14

#Optimization flags
#
win32 {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
    CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
    CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
    CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
    LIBS += -L$$DESTDIR -lprotobuf

    INCLUDEPATH += ./../Protobuf
}
unix {
    LIBS += -lprotobuf
}

#QScintilla
#
INCLUDEPATH += ./../QScintilla/Qt4Qt5
DEPENDPATH += ./../QScintilla/Qt4Qt5
LIBS += -L$$DESTDIR -lQScintilla
win32:PRE_TARGETDEPS += $$DESTDIR/QScintilla.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libQScintilla.a

# Add curent dir to a list of library directory paths
#
#QMAKE_RPATHDIR += ./
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
    CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
    CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# QtPropertyBrowser
#
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

# QtKeychain
#
win32 {
    LIBS += Advapi32.lib
}

DEFINES += QTKEYCHAIN_NO_EXPORT
DEFINES += USE_CREDENTIAL_STORE

INCLUDEPATH += ./qtkeychain-0.8

include(../Tools/qtkeychain-0.8/qt5keychain.pri)

