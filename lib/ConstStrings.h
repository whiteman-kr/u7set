#pragma once

namespace EquipmentPropNames
{
	// Ethernet controller properties
	//
	inline const QString LAN_CONTROLLER_TYPE("LanControllerType");
	inline const QString CONTROLLER_NO("ControllerNo");				// equals to Place

	inline const QString TUNING_ENABLE("TuningEnable");
	inline const QString TUNING_IP("TuningIP");
	inline const QString TUNING_PORT("TuningPort");
	inline const QString TUNING_SERVICE_ID("TuningServiceID");
	inline const QString TUNING_SERVICE_IP("TuningServiceIP");
	inline const QString TUNING_SERVICE_PORT("TuningServicePort");
	inline const QString TUNING_SERVICE_NETMASK("TuningServiceNetmask");
	inline const QString TUNING_SOURCE_EQUIPMENT_ID("TuningSourceEquipmentID");

	inline const QString APP_DATA_ENABLE("AppDataEnable");
	inline const QString APP_DATA_IP("AppDataIP");
	inline const QString APP_DATA_PORT("AppDataPort");
	inline const QString APP_DATA_SERVICE_ID("AppDataServiceID");
	inline const QString APP_DATA_SERVICE_IP("AppDataServiceIP");
	inline const QString APP_DATA_SERVICE_PORT("AppDataServicePort");
	inline const QString APP_DATA_SERVICE_NETMASK("AppDataServiceNetmask");
	inline const QString APP_DATA_SIZE_BYTES("AppDataSizeBytes");
	inline const QString APP_DATA_UID("AppDataUID");
	inline const QString HEX_APP_DATA_UID("HexAppDataUID");
	inline const QString APP_DATA_FRAMES_QUANTITY("AppDataFramesQuantity");
	inline const QString OVERRIDE_APP_DATA_WORD_COUNT("OverrideAppDataWordCount");

	inline const QString DIAG_DATA_ENABLE("DiagDataEnable");
	inline const QString DIAG_DATA_IP("DiagDataIP");
	inline const QString DIAG_DATA_PORT("DiagDataPort");
	inline const QString DIAG_DATA_SERVICE_ID("DiagDataServiceID");
	inline const QString DIAG_DATA_SERVICE_IP("DiagDataServiceIP");
	inline const QString DIAG_DATA_SERVICE_PORT("DiagDataServicePort");
	inline const QString DIAG_DATA_SERVICE_NETMASK("DiagDataServiceNetmask");
	inline const QString DIAG_DATA_SIZE_BYTES("DiagDataSizeBytes");
	inline const QString DIAG_DATA_UID("DiagDataUID");
	inline const QString HEX_DIAG_DATA_UID("HexDiagDataUID");
	inline const QString DIAG_DATA_FRAMES_QUANTITY("DiagDataFramesQuantity");
	inline const QString OVERRIDE_DIAG_DATA_WORD_COUNT("OverrideDiagDataWordCount");

	// LM properties
	//
	inline const QString LM_APP_LAN_DATA_UID("AppLANDataUID");
	inline const QString LM_APP_LAN_DATA_SIZE("AppLANDataSize");

	inline const QString LM_DIAG_LAN_DATA_UID("DiagLANDataUID");
	inline const QString LM_DIAG_LAN_DATA_SIZE("DiagLANDataSize");

	inline const QString EQUIPMENT_ID("EquipmentID");
	inline const QString CAPTION("Caption");

	inline const QString SUBSYSTEM_ID("SubsystemID");
	inline const QString SUBSYSTEM_KEY("SubsystemKey");
	inline const QString LM_NUMBER("LMNumber");
	inline const QString SUBSYSTEM_CHANNEL("SubsystemChannel");
	inline const QString LM_UNIQUE_ID("LmUniqueID");
	inline const QString LM_EQUIPMENT_ID("LmEquipmentID");
	inline const QString PORT_EQUIPMENT_ID("PortEquipmentID");

	inline const QString MODULE_FAMILY("ModuleFamily");
	inline const QString MODULE_FAMILY_ID("ModuleFamilyID");
	inline const QString MODULE_VERSION("ModuleVersion");

	inline const QString PRESET_NAME("PresetName");
	inline const QString LM_DESCRIPTION_FILE("LmDescriptionFile");

	// TuningService properties
	//
	inline const QString TUNING_DATA_NETMASK("TuningDataNetmask");
	inline const QString TUNING_DATA_IP("TuningDataIP");
	inline const QString TUNING_DATA_PORT("TuningDataPort");
	inline const QString SINGLE_LM_CONTROL("SingleLmControl");
	inline const QString DISABLE_MODULES_TYPE_CHECKING("DisableModulesTypeChecking");

	inline const QString TUNING_SIM_IP("TunigSimIP");
	inline const QString TUNING_SIM_PORT("TunigSimPort");

	// TuningClient properties
	//
	inline const QString AUTO_APPLAY("AutoApply");
	inline const QString SHOW_SIGNALS("ShowSignals");
	inline const QString SHOW_SCHEMAS("ShowSchemas");
	inline const QString SCHEMAS_NAVIGATION("SchemasNavigation");
	inline const QString SHOW_SCHEMAS_LIST("ShowSchemasList");
	inline const QString SHOW_SCHEMAS_TABS("ShowSchemasTabs");
	inline const QString STATUS_FLAG_FUNCTION("StatusFlagFunction");
	inline const QString SHOW_SOR("ShowSOR");
	inline const QString USE_ACCESS_FLAG("UseAccessFlag");
	inline const QString LOGIN_PER_OPERATION("LoginPerOperation");
	inline const QString USER_ACCOUNTS("UsersAccounts");
	inline const QString LOGIN_SESSION_LENGTH("LoginSessionLength");
	inline const QString FILTER_BY_EQUIPMENT("FilterByEquipment");
	inline const QString FILTER_BY_SCHEMA("FilterBySchema");

	// AppDataService properties
	//
	inline const QString APP_DATA_RECEIVING_NETMASK("AppDataReceivingNetmask");
	inline const QString APP_DATA_RECEIVING_IP("AppDataReceivingIP");
	inline const QString APP_DATA_RECEIVING_PORT("AppDataReceivingPort");
	inline const QString RT_TRENDS_REQUEST_IP("RtTrendsRequestIP");
	inline const QString RT_TRENDS_REQUEST_PORT("RtTrendsRequestPort");
	inline const QString ARCH_SERVICE_ID("ArchiveServiceID");
	inline const QString ARCH_SERVICE_IP("ArchiveServiceIP");
	inline const QString ARCH_SERVICE_PORT("ArchiveServicePort");
	inline const QString AUTO_ARCHIVE_INTERVAL("AutoArchiveInterval");

	// DiagDataService properties
	//
	inline const QString DIAG_DATA_RECEIVING_NETMASK("DiagDataReceivingNetmask");
	inline const QString DIAG_DATA_RECEIVING_IP("DiagDataReceivingIP");
	inline const QString DIAG_DATA_RECEIVING_PORT("DiagDataReceivingPort");

	// ArchivingService properties
	//
	inline const QString ARCHIVE_SHORT_TERM_PERIOD("ShortTermArchivePeriod");
	inline const QString ARCHIVE_LONG_TERM_PERIOD("LongTermArchivePeriod");
	inline const QString ARCHIVE_LOCATION("ArchiveLocation");

	// Properties used in several Services
	//
	inline const QString CLIENT_REQUEST_IP("ClientRequestIP");
	inline const QString CLIENT_REQUEST_NETMASK("ClientRequestNetmask");
	inline const QString CLIENT_REQUEST_PORT("ClientRequestPort");

	inline const QString CFG_SERVICE_ID1("ConfigurationServiceID1");
	inline const QString CFG_SERVICE_IP1("ConfigurationServiceIP1");
	inline const QString CFG_SERVICE_PORT1("ConfigurationServicePort1");

	inline const QString CFG_SERVICE_ID2("ConfigurationServiceID2");
	inline const QString CFG_SERVICE_IP2("ConfigurationServiceIP2");
	inline const QString CFG_SERVICE_PORT2("ConfigurationServicePort2");

	inline const QString SOFTWARE_TYPE("SoftwareType");

	inline const QString APP_DATA_SERVICE_ID1("AppDataServiceID1");
	inline const QString APP_DATA_SERVICE_ID2("AppDataServiceID2");

	inline const QString START_SCHEMA_ID("StartSchemaID");
	inline const QString SCHEMA_TAGS("SchemaTags");
}

namespace XmlElement
{
	inline const QString SOFTWARE_ITEMS("SoftwareItems");
	inline const QString SOFTWARE("Software");
	inline const QString SETTINGS("Settings");
	inline const QString CLIENTS("Clients");
	inline const QString CLIENT("Client");

	inline const QString CFG_SERVICE1("CfgService1");
	inline const QString CFG_SERVICE2("CfgService2");

	inline const QString APP_DATA_SERVICE("AppDataService");
	inline const QString APP_DATA_SERVICE1("AppDataService1");
	inline const QString APP_DATA_SERVICE2("AppDataService2");

	inline const QString ARCHIVE_SERVICE("ArchiveService");
	inline const QString ARCHIVE_SERVICE1("ArchiveService1");
	inline const QString ARCHIVE_SERVICE2("ArchiveService2");

	inline const QString DIAG_DATA_SERVICE("DiagDataService");
	inline const QString TUNING_SERVICE("TuningService");

	inline const QString APPEARANCE("Appearance");

	inline const QString TUNING_CLIENTS("TuningClients");
	inline const QString TUNING_CLIENT("TuningClient");
	inline const QString TUNING_SOURCES("TuningSources");
	inline const QString TUNING_SOURCE("TuningSource");

	inline const QString DATA_SOURCES("DataSources");
	inline const QString DATA_SOURCE("DataSource");
	inline const QString ASSOCIATED_SIGNALS("AssociatedSignals");

	inline const QString SETTINGS_SET("SettingsSet");
}

namespace XmlAttribute
{
	inline const QString CAPTION("Caption");
	inline const QString COUNT("Count");
	inline const QString ID("ID");
	inline const QString TYPE("Type");

	inline const QString DATA_ID("DataID");
	inline const QString HEX_DATA_ID("HexDataID");

	inline const QString PROFILE("Profile");

	inline const QString APP_DATA_SERVICE_PROPERTY_IS_VALID1("AppDataServicePropertyIsValid1");
	inline const QString APP_DATA_SERVICE_ID1("AppDataServiceID1");
	inline const QString APP_DATA_SERVICE_IP1("AppDataServiceIP1");
	inline const QString APP_DATA_SERVICE_PORT1("AppDataServicePort1");
	inline const QString REALTIME_DATA_IP1("RealtimeDataIP1");
	inline const QString REALTIME_DATA_PORT1("RealtimeDataPort1");

	inline const QString APP_DATA_SERVICE_PROPERTY_IS_VALID2("AppDataServicePropertyIsValid2");
	inline const QString APP_DATA_SERVICE_ID2("AppDataServiceID2");
	inline const QString APP_DATA_SERVICE_IP2("AppDataServiceIP2");
	inline const QString APP_DATA_SERVICE_PORT2("AppDataServicePort2");
	inline const QString REALTIME_DATA_IP2("RealtimeDataIP2");
	inline const QString REALTIME_DATA_PORT2("RealtimeDataPort2");

	inline const QString TUNING_SERVICE_PROPERTY_IS_VALID("TuningServicePropertyIsValid");
	inline const QString TUNING_SERVICE_IP("TuningServiceIP");
	inline const QString TUNING_SERVICE_PORT("TuningServicePort");

	inline const QString SOFTWARE_METROLOGY_ID("SoftwareMetrologyID");

	inline const QString LM_DATA_TYPE("LmDataType");
	inline const QString LM_ID("LmEquipmentID");
	inline const QString LM_PRESET_NAME("LmPresetName");
	inline const QString LM_NUMBER("LmNumber");
	inline const QString LM_CHANNEL("LmChannel");
	inline const QString LM_SUBSYSTEM_KEY("LmSubsystemKey");
	inline const QString LM_SUBSYSTEM_ID("LmSubsystemID");
	inline const QString LM_MODULE_TYPE("LmModuleType");
	inline const QString LM_CAPTION("LmCaption");
	inline const QString LM_ADAPTER_ID("LmAdapterID");
	inline const QString LM_DATA_ENABLE("LmDataEnable");
	inline const QString LM_DATA_IP("LmDataIP");
	inline const QString LM_DATA_PORT("LmDataPort");
	inline const QString LM_DATA_SIZE("LmDataSize");
	inline const QString LM_RUP_FRAMES_QUANTITY("LmRupFramesQuantity");
	inline const QString LM_DATA_ID("LmDataID");
	inline const QString LM_UNIQUE_ID("LmUniqueID");
	inline const QString SERVICE_ID("ServiceID");
}

namespace Latin1Char
{
	inline const QLatin1Char ZERO('0');
	inline const QLatin1Char SPACE(' ');
}

namespace CfgFileId
{
	inline const QString APP_DATA_SOURCES("APP_DATA_SOURCES");
	inline const QString APP_SIGNALS("APP_SIGNALS");
	inline const QString APP_SIGNAL_SET("APP_SIGNAL_SET");
	inline const QString COMPARATOR_SET("COMPARATOR_SET");
	inline const QString UNIT_SET("UNIT_SET");

	inline const QString TUNING_SOURCES("TUNING_SOURCES");
	inline const QString TUNING_SIGNALS("TUNING_SIGNALS");
	inline const QString TUNING_SCHEMAS_DETAILS("TUNING_SCHEMAS_DETAILS");
	inline const QString TUNING_FILTERS("TUNING_FILTERS");
	inline const QString TUNING_GLOBALSCRIPT("TUNING_GLOBALSCRIPT");
	inline const QString TUNING_CONFIGARRIVEDSCRIPT("TUNING_CONFIGARRIVEDSCRIPT");

	inline const QString CLIENT_BEHAVIOR("CLIENT_BEHAVIOR");
	inline const QString LOGO("LOGO");

	inline const QString METROLOGY_ITEMS("METROLOGY_ITEMS");
	inline const QString METROLOGY_SIGNAL_SET("METROLOGY_SIGNAL_SET");
}

namespace  SettingsProfile
{
	inline const QString DEFAULT("Default");
}

namespace Directory
{
	inline const QString COMMON("Common");

	inline const QString REPORTS("Reports");
	inline const QString OPTO_VHD("Opto-vhd");
	inline const QString RUN_SERVICE_SCRIPTS("RunServiceScripts");
	inline const QString BIN("Bin");
}

namespace File
{
	inline const QString APP_SIGNALS_ASGS("AppSignals.asgs");
	inline const QString COMPARATORS_SET("Comparators.set");
	inline const QString APP_DATA_SOURCES_XML("AppDataSources.xml");
	inline const QString TUNING_SOURCES_XML("TuningSources.xml");
	inline const QString CONFIGURATION_XML("Configuration.xml");
	inline const QString SOFTWARE_XML("Software.xml");
	inline const QString SUBSYSTEMS_XML("Subsystems.xml");

	inline const QString METROLOGY_ITEMS_XML("MetrologyItems.xml");
	inline const QString METROLOGY_SIGNAL_SET("MetrologySignal.set");

	inline const QString CONNECTIONS_TXT("Connections.txt");
	inline const QString CONNECTIONS_XML("Connections.xml");
	inline const QString LOGIC_MODULES_XML("LogicModules.xml");
}

namespace Separator
{
	inline const QString SEMICOLON_SPACE("; ");
	inline const QString SEMICOLON(";");
	inline const QString LINE("-------------------------------------------------------------------------------");
}
