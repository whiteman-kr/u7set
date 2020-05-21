#-------------------------------------------------
#
# Project created by QtCreator 2013-03-06T04:51:03
#
#-------------------------------------------------

QT       += core gui widgets sql network xmlpatterns qml svg serialport xml printsupport testlib

# --
# In Qt 5 using testlib module adds a console option via the MODULE_CONFIG mechanism.
# This forces a /SUBSYSTEM: CONSOLE onto the linker command line no matter what global
# options you specify, even if you use CONFIG -= console.
# The console configuration is given in the testlib module configuration within
# qtbase/src/testlib/testlib.pro. This means that it ends up in QT.testlib.CONFIG variable.

QT.testlib.CONFIG -= console

# --

TARGET = u7
TEMPLATE = app

win32:LIBS += -lGdi32

INCLUDEPATH += $$PWD

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet
win32:QMAKE_CXXFLAGS += /analyze		# Static code analyze

# Warning level
#
gcc:CONFIG += warn_on

win32:CONFIG -= warn_on				# warn_on is level 3 warnings
win32:QMAKE_CXXFLAGS += /W4			# CONFIG += warn_on is just W3 level, so set level 4
win32:QMAKE_CXXFLAGS += /wd4201		# Disable warning: C4201: nonstandard extension used: nameless struct/union
win32:QMAKE_CXXFLAGS += /wd4458		# Disable warning: C4458: declaration of 'selectionPen' hides class member
win32:QMAKE_CXXFLAGS += /wd4275		# Disable warning: C4275: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'

#Application icon
win32:RC_ICONS += Images/u7.ico

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


SOURCES +=\
    ../lib/ClientBehavior.cpp \
    ../lib/ExportPrint.cpp \
    ../lib/Ui/DialogSignalInfo.cpp \
    ../lib/Ui/DialogSignalSearch.cpp \
    ../lib/Ui/DialogSignalSnapshot.cpp \
    ../lib/Ui/TabWidgetEx.cpp \
    ../lib/Ui/TagSelectorWidget.cpp \
    CentralWidget.cpp \
    CreateProjectDialog.cpp \
    CreateUserDialogDialog.cpp \
    DialogClientBehavior.cpp \
    DialogSettings.cpp \
    EditEngine/EditEngineNop.cpp \
    FilesTabPage.cpp \
    LoginDialog.cpp \
    Main.cpp \
    MainTabPage.cpp \
    MainWindow.cpp \
    PasswordService.cpp \
    Settings.cpp \
    Simulator/SimAppLogicSchemasPage.cpp \
    Simulator/SimConnectionPage.cpp \
    Simulator/SimLogicModulePage.cpp \
    Simulator/SimOverridePane.cpp \
    Simulator/SimOverrideValueWidget.cpp \
    Simulator/SimSignalInfo.cpp \
    Simulator/SimSignalSnapshot.cpp \
    Simulator/SimTrend/SimTrends.cpp \
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
    ../lib/ModuleFirmware.cpp \
    BuildTabPage.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbProgress.cpp \
    ../lib/Crc.cpp \
    DialogFileEditor.cpp \
    DialogSubsystemListEditor.cpp \
    EquipmentVcsDialog.cpp \
    ../lib/DataSource.cpp \
    ../lib/SocketIO.cpp \
    ../lib/PropertyEditor.cpp \
    ../lib/Types.cpp \
    ../lib/PropertyEditorDialog.cpp \
    ../lib/BuildInfo.cpp \
    GlobalMessanger.cpp \
    EditSchemaWidget.cpp \
    SchemaPropertiesDialog.cpp \
    SchemaItemPropertiesDialog.cpp \
    SchemaLayersDialog.cpp \
    CreateSchemaDialog.cpp \
    EditEngine/EditEngineSetSchemaProperty.cpp \
    EditEngine/EditEngineSetOrder.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/Queue.cpp \
    UploadTabPage.cpp \
    DialogChoosePreset.cpp \
    ../lib/WUtils.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Configurator.cpp \
    DialogSettingsConfigurator.cpp \
    Forms/ChooseUfbDialog.cpp \
    Forms/SelectChangesetDialog.cpp \
    Forms/FileHistoryDialog.cpp \
    Forms/ChangesetDetailsDialog.cpp \
    Forms/CompareDialog.cpp \
    Forms/ComparePropertyObjectDialog.cpp \
    ../TuningIPEN/TuningIPENDataStorage.cpp \
    DialogConnections.cpp \
    ../lib/MetrologySignal.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/Tuning/TuningFilterEditor.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/Tuning/TuningModel.cpp \
    ../lib/AppSignal.cpp \
    ../lib/CsvFile.cpp \
    DialogBusEditor.cpp \
    BusStorage.cpp \
    Forms/DialogUpdateFromPreset.cpp \
    Forms/ChooseAfbDialog.cpp \
    IdePropertyEditor.cpp \
    EditEngine/EditEngineSetObject.cpp \
    ../lib/Address16.cpp \
    EditConnectionLine.cpp \
    EditEngine/EditEngineBatch.cpp \
    CreateSignalDialog.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../Proto/network.pb.cc \
    ../lib/LmDescription.cpp \
    SimulatorTabPage.cpp \
    Simulator/SimIdeSimulator.cpp \
    ../lib/TuningValue.cpp \
    Simulator/SimSchemaWidget.cpp \
    Simulator/SimSchemaManager.cpp \
    Simulator/SimSchemaView.cpp \
    Simulator/SimTuningTcpClient.cpp \
    ../lib/AppSignalManager.cpp \
    Simulator/SimCodePage.cpp \
    Simulator/SimWidget.cpp \
    Simulator/SimSelectBuildDialog.cpp \
    Simulator/SimSchemaPage.cpp \
    Simulator/SimProjectWidget.cpp \
    Simulator/SimOutputWidget.cpp \
    Simulator/SimMemoryWidget.cpp \
    Simulator/SimBasePage.cpp \
    ../lib/Times.cpp \
    SpecificPropertiesEditor.cpp \
    ../lib/Ui/DialogAbout.cpp \
    ../lib/Subsystem.cpp \
    ../lib/Connection.cpp \
    ../lib/LogicModuleSet.cpp \
    SchemaTabPageEx.cpp \
    DialogInputEx.cpp \
    DialogAfbLibraryCheck.cpp \
    ../lib/WidgetUtils.cpp \
    Forms/ProjectPropertiesForm.cpp \
    Forms/PendingChangesDialog.cpp \
    ../lib/SimpleMutex.cpp \
    ../lib/Ui/TextEditCompleter.cpp \
    ../lib/QScintillaLexers/LexerJavaScript.cpp \
    ../lib/QScintillaLexers/LexerXML.cpp \
    DialogShortcuts.cpp \
    ../lib/Ui/UiTools.cpp \
    SvgEditor.cpp \
    ../lib/PropertyTable.cpp


HEADERS  += \
    ../lib/ClientBehavior.h \
    ../lib/ExportPrint.h \
    ../lib/StandardColors.h \
    ../lib/Ui/DialogSignalInfo.h \
    ../lib/Ui/DialogSignalSearch.h \
    ../lib/Ui/DialogSignalSnapshot.h \
    ../lib/Ui/TabWidgetEx.h \
    ../lib/Ui/TagSelectorWidget.h \
    CentralWidget.h \
    CreateProjectDialog.h \
    CreateUserDialogDialog.h \
    DialogClientBehavior.h \
    DialogSettings.h \
    EditEngine/EditEngineNop.h \
    FilesTabPage.h \
    LoginDialog.h \
    MainTabPage.h \
    MainWindow.h \
    PasswordService.h \
    Settings.h \
    Simulator/SimAppLogicSchemasPage.h \
    Simulator/SimConnectionPage.h \
    Simulator/SimLogicModulePage.h \
    Simulator/SimOverridePane.h \
    Simulator/SimOverrideValueWidget.h \
    Simulator/SimSignalInfo.h \
    Simulator/SimSignalSnapshot.h \
    Simulator/SimTrend/SimTrends.h \
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
    ../lib/ModuleFirmware.h \
    BuildTabPage.h \
    ../lib/OutputLog.h \
    ../lib/DbProgress.h \
    ../lib/Crc.h \
    DialogFileEditor.h \
    DialogSubsystemListEditor.h \
    ../lib/Types.h \
    Forms/ChooseAfbDialog.h \
    EquipmentVcsDialog.h \
    ../lib/DataSource.h \
    ../lib/SocketIO.h \
    ../lib/PropertyObject.h \
    ../lib/PropertyEditor.h \
    ../lib/PropertyEditorDialog.h \
    ../lib/DebugInstCounter.h \
    ../lib/BuildInfo.h \
    GlobalMessanger.h \
    ../lib/Address16.h \
    ../lib/WUtils.h \
    EditSchemaWidget.h \
    SchemaPropertiesDialog.h \
    SchemaItemPropertiesDialog.h \
    SchemaLayersDialog.h \
    CreateSchemaDialog.h \
    EditEngine/EditEngineSetSchemaProperty.h \
    EditEngine/EditEngineSetOrder.h \
    ../lib/DeviceHelper.h \
    ../lib/XmlHelper.h \
    ../lib/ServiceSettings.h \
    ../lib/Queue.h \
    UploadTabPage.h \
    DialogChoosePreset.h \
    ../lib/DataProtocols.h \
    ../lib/HostAddressPort.h \
    ../lib/SignalProperties.h \
    ../lib/Configurator.h \
    DialogSettingsConfigurator.h \
    Forms/ChooseUfbDialog.h \
    ../lib/LmLimits.h \
    Forms/SelectChangesetDialog.h \
    Forms/FileHistoryDialog.h \
    Forms/ChangesetDetailsDialog.h \
    Forms/CompareDialog.h \
    Forms/ComparePropertyObjectDialog.h \
    ../lib/diff_match_patch.h \
    ../TuningIPEN/TuningIPENDataStorage.h \
    DialogConnections.h \
    ../lib/MetrologySignal.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/Tuning/TuningFilterEditor.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/TuningModel.h \
    ../lib/AppSignal.h \
    ../lib/CsvFile.h \
    ../lib/WidgetUtils.h \
    DialogBusEditor.h \
    BusStorage.h \
    Forms/DialogUpdateFromPreset.h \
    IdePropertyEditor.h \
    EditEngine/EditEngineSetObject.h \
    EditConnectionLine.h \
    EditEngine/EditEngineBatch.h \
    CreateSignalDialog.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../Proto/network.pb.h \
    ../lib/LmDescription.h \
    SimulatorTabPage.h \
    Simulator/SimIdeSimulator.h \
    ../lib/TuningValue.h \
    Simulator/SimSchemaWidget.h \
    Simulator/SimSchemaManager.h \
    Simulator/SimSchemaView.h \
    Simulator/SimTuningTcpClient.h \
    ../lib/AppSignalManager.h \
    Simulator/SimCodePage.h \
    Simulator/SimWidget.h \
    Simulator/SimSelectBuildDialog.h \
    Simulator/SimSchemaPage.h \
    Simulator/SimBasePage.h \
    Simulator/SimMemoryWidget.h \
    Simulator/SimOutputWidget.h \
    Simulator/SimProjectWidget.h \
    SpecificPropertiesEditor.h \
    ../lib/CommonTypes.h \
    ../lib/Times.h \
    ../lib/Ui/DialogAbout.h \
    ../lib/Subsystem.h \
    ../lib/DbObjectStorage.h \
    ../lib/Connection.h \
    ../lib/LogicModuleSet.h \
    SchemaTabPageEx.h \
    DialogInputEx.h \
    DialogAfbLibraryCheck.h \
    Forms/ProjectPropertiesForm.h \
    Forms/PendingChangesDialog.h \
    ../lib/SimpleMutex.h \
    ../lib/Ui/TextEditCompleter.h \
    ../lib/QScintillaLexers/LexerJavaScript.h \
    ../lib/QScintillaLexers/LexerXML.h \
    DialogShortcuts.h \
    ../lib/Ui/UiTools.h \
    SvgEditor.h \
    ../lib/PropertyTable.h


FORMS    += \
    ../lib/Ui/DialogSignalInfo.ui \
    CreateProjectDialog.ui \
    CreateUserDialogDialog.ui \
    DialogSettings.ui \
    LoginDialog.ui \
    Simulator/SimSelectBuildDialog.ui \
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
    ../Proto/proto_compile.sh

DISTFILES += \
    ../Etc/ClientBehavior/ClientBehavior.xml \
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
    LogicModuleDescription/MSO4_SR21.xml \
    Ufbl/UFB_A3_LANDSCAPE.templ_ufb \
    Ufbl/UFB_A4_LANDSCAPE.templ_ufb \
    Ufbl/file2pgsql.exe \
    Ufbl/_convert_all.bat \
    LogicModuleDescription/LogicModule0000.xml \
    ../Proto/network.proto \
    LogicModuleDescription/LM1_SF00.xml \
    LogicModuleDescription/LM1_SR01.xml \
    LogicModuleDescription/LM1_SF00.xml \
    LogicModuleDescription/LM1_SR01.xml \
    LogicModuleDescription/BVB15Module0000.xml \
    LogicModuleDescription/LM1_SR02.xml \
    ../Etc/SignalPropertyBehavior/SignalPropertyBehavior.csv \
    LogicModuleDescription/MSO3Module0000.xml \
    Images/u7.ico \
    LogicModuleDescription/LM1_SR04.xml \
    LogicModuleDescription/LM1_SR03.xml \
    LogicModuleDescription/LM1_SF40.xml

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

win32 {
    #CONFIG(debug, debug|release): DEFINES += _CRTDBG_MAP_ALLOC
    #CONFIG(debug, debug|release): DEFINES += "DBG_NEW=new(_NORMAL_BLOCK,__FILE__,__LINE__)"
    #CONFIG(debug, debug|release): DEFINES += "new=DBG_NEW"
}



CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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

# QScintilla
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

# Builder Lib
#
INCLUDEPATH += $$PWD/../Builder
DEPENDPATH += $$PWD/../Builder

win32 {
    LIBS += -L$$DESTDIR -lBuilder

    CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/debug/Builder.lib
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/release/Builder.lib
}
unix {
    LIBS += -lBuilder
}

# QtPropertyBrowser
#
include(../qtpropertybrowser/src/qtpropertybrowser.pri)

# QtKeychain
#
win32 {
    LIBS += Advapi32.lib

    DEFINES += QTKEYCHAIN_NO_EXPORT
    DEFINES += USE_CREDENTIAL_STORE

    INCLUDEPATH += ./qtkeychain-0.9.1

    include(../Tools/qtkeychain-0.9.1/qt5keychain.pri)
}
unix {
    LIBS += -lqtkeychain
}

# Simulator Lib
#
INCLUDEPATH += $$PWD/../Simulator
DEPENDPATH += $$PWD/../Simulator

win32 {
    LIBS += -L$$DESTDIR -lSimulator

    CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/debug/Simulator.lib
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../bin/release/Simulator.lib
}
unix {
    LIBS += -lSimulator
}

# TrendView library
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TrendView/release/ -lTrendView
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TrendView/debug/ -lTrendView
else:unix:!macx: LIBS += -L$$OUT_PWD/../TrendView/ -lTrendView

INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/libTrendView.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/libTrendView.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/TrendView.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/TrendView.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../TrendView/libTrendView.a
