#pragma once

namespace EquipmentPropNames
{
	// Ethernet controller properties
	//
	const QString LAN_CONTROLLER_TYPE("LanControllerType");
	const QString CONTROLLER_NO("ControllerNo");				// equals to Place

	const QString TUNING_ENABLE("TuningEnable");
	const QString TUNING_IP("TuningIP");
	const QString TUNING_PORT("TuningPort");
	const QString TUNING_SERVICE_ID("TuningServiceID");
	const QString TUNING_SERVICE_IP("TuningServiceIP");
	const QString TUNING_SERVICE_PORT("TuningServicePort");
	const QString TUNING_SERVICE_NETMASK("TuningServiceNetmask");
	const QString TUNING_SOURCE_EQUIPMENT_ID("TuningSourceEquipmentID");

	const QString APP_DATA_ENABLE("AppDataEnable");
	const QString APP_DATA_IP("AppDataIP");
	const QString APP_DATA_PORT("AppDataPort");
	const QString APP_DATA_SERVICE_ID("AppDataServiceID");
	const QString APP_DATA_SERVICE_IP("AppDataServiceIP");
	const QString APP_DATA_SERVICE_PORT("AppDataServicePort");
	const QString APP_DATA_SERVICE_NETMASK("AppDataServiceNetmask");
	const QString APP_DATA_SIZE_BYTES("AppDataSizeBytes");
	const QString APP_DATA_UID("AppDataUID");
	const QString HEX_APP_DATA_UID("HexAppDataUID");
	const QString APP_DATA_FRAMES_QUANTITY("AppDataFramesQuantity");
	const QString OVERRIDE_APP_DATA_WORD_COUNT("OverrideAppDataWordCount");

	const QString DIAG_DATA_ENABLE("DiagDataEnable");
	const QString DIAG_DATA_IP("DiagDataIP");
	const QString DIAG_DATA_PORT("DiagDataPort");
	const QString DIAG_DATA_SERVICE_ID("DiagDataServiceID");
	const QString DIAG_DATA_SERVICE_IP("DiagDataServiceIP");
	const QString DIAG_DATA_SERVICE_PORT("DiagDataServicePort");
	const QString DIAG_DATA_SERVICE_NETMASK("DiagDataServiceNetmask");
	const QString DIAG_DATA_SIZE_BYTES("DiagDataSizeBytes");
	const QString DIAG_DATA_UID("DiagDataUID");
	const QString HEX_DIAG_DATA_UID("HexDiagDataUID");
	const QString DIAG_DATA_FRAMES_QUANTITY("DiagDataFramesQuantity");
	const QString OVERRIDE_DIAG_DATA_WORD_COUNT("OverrideDiagDataWordCount");

	// LM properties
	//
	const QString LM_APP_LAN_DATA_UID("AppLANDataUID");
	const QString LM_APP_LAN_DATA_SIZE("AppLANDataSize");

	const QString LM_DIAG_LAN_DATA_UID("DiagLANDataUID");
	const QString LM_DIAG_LAN_DATA_SIZE("DiagLANDataSize");

	const QString EQUIPMENT_ID("EquipmentID");
	const QString CAPTION("Caption");

	const QString SUBSYSTEM_ID("SubsystemID");
	const QString LM_NUMBER("LMNumber");
	const QString SUBSYSTEM_CHANNEL("SubsystemChannel");

	const QString MODULE_FAMILY("ModuleFamily");
	const QString MODULE_FAMILY_ID("ModuleFamilyID");
	const QString MODULE_VERSION("ModuleVersion");

	const QString PRESET_NAME("PresetName");
	const QString LM_DESCRIPTION_FILE("LmDescriptionFile");

	// TuningService properties
	//
	const QString TUNING_DATA_NETMASK("TuningDataNetmask");
	const QString TUNING_DATA_IP("TuningDataIP");
	const QString TUNING_DATA_PORT("TuningDataPort");
	const QString SINGLE_LM_CONTROL("SingleLmControl");
	const QString DISABLE_MODULES_TYPE_CHECKING("DisableModulesTypeChecking");

	// TuningClient properties
	//
	const QString AUTO_APPLAY("AutoApply");
	const QString SHOW_SIGNALS("ShowSignals");
	const QString SHOW_SCHEMAS("ShowSchemas");
	const QString SCHEMAS_NAVIGATION("SchemasNavigation");
	const QString SHOW_SCHEMAS_LIST("ShowSchemasList");
	const QString SHOW_SCHEMAS_TABS("ShowSchemasTabs");
	const QString STATUS_FLAG_FUNCTION("StatusFlagFunction");
	const QString SHOW_SOR("ShowSOR");
	const QString USE_ACCESS_FLAG("UseAccessFlag");
	const QString LOGIN_PER_OPERATION("LoginPerOperation");
	const QString USER_ACCOUNTS("UsersAccounts");
	const QString LOGIN_SESSION_LENGTH("LoginSessionLength");
	const QString FILTER_BY_EQUIPMENT("FilterByEquipment");
	const QString FILTER_BY_SCHEMA("FilterBySchema");

	// AppDataService properties
	//
	const QString APP_DATA_RECEIVING_NETMASK("AppDataReceivingNetmask");
	const QString APP_DATA_RECEIVING_IP("AppDataReceivingIP");
	const QString APP_DATA_RECEIVING_PORT("AppDataReceivingPort");
	const QString RT_TRENDS_REQUEST_IP("RtTrendsRequestIP");
	const QString RT_TRENDS_REQUEST_PORT("RtTrendsRequestPort");
	const QString ARCH_SERVICE_ID("ArchiveServiceID");
	const QString ARCH_SERVICE_IP("ArchiveServiceIP");
	const QString ARCH_SERVICE_PORT("ArchiveServicePort");
	const QString AUTO_ARCHIVE_INTERVAL("AutoArchiveInterval");

	// DiagDataService properties
	//
	const QString DIAG_DATA_RECEIVING_NETMASK("DiagDataReceivingNetmask");
	const QString DIAG_DATA_RECEIVING_IP("DiagDataReceivingIP");
	const QString DIAG_DATA_RECEIVING_PORT("DiagDataReceivingPort");

	// ArchivingService properties
	//
	const QString ARCHIVE_SHORT_TERM_PERIOD("ShortTermArchivePeriod");
	const QString ARCHIVE_LONG_TERM_PERIOD("LongTermArchivePeriod");
	const QString ARCHIVE_LOCATION("ArchiveLocation");

	// Properties used in several Services
	//
	const QString CLIENT_REQUEST_IP("ClientRequestIP");
	const QString CLIENT_REQUEST_NETMASK("ClientRequestNetmask");
	const QString CLIENT_REQUEST_PORT("ClientRequestPort");

	const QString CFG_SERVICE_ID1("ConfigurationServiceID1");
	const QString CFG_SERVICE_IP1("ConfigurationServiceIP1");
	const QString CFG_SERVICE_PORT1("ConfigurationServicePort1");

	const QString CFG_SERVICE_ID2("ConfigurationServiceID2");
	const QString CFG_SERVICE_IP2("ConfigurationServiceIP2");
	const QString CFG_SERVICE_PORT2("ConfigurationServicePort2");

	const QString SOFTWARE_TYPE("SoftwareType");

	const QString APP_DATA_SERVICE_ID1("AppDataServiceID1");
	const QString APP_DATA_SERVICE_ID2("AppDataServiceID2");

	const QString START_SCHEMA_ID("StartSchemaID");
	const QString SCHEMA_TAGS("SchemaTags");
}

namespace XmlElement
{
	const QString SOFTWARE_ITEMS("SoftwareItems");
	const QString SOFTWARE("Software");
	const QString SETTINGS("Settings");
	const QString CLIENTS("Clients");
	const QString CLIENT("Client");

	const QString APP_DATA_SERVICE("AppDataService");
	const QString APP_DATA_SERVICE1("AppDataService1");
	const QString APP_DATA_SERVICE2("AppDataService2");

	const QString ARCHIVE_SERVICE1("ArchiveService1");
	const QString ARCHIVE_SERVICE2("ArchiveService2");

	const QString DIAG_DATA_SERVICE("DiagDataService");
	const QString TUNING_SERVICE("TuningService");

	const QString APPEARANCE("Appearance");
}

namespace XmlAttribute
{
	const QString CAPTION("Caption");
	const QString COUNT("Count");
	const QString ID("ID");
	const QString TYPE("Type");

	const QString DATA_ID("DataID");
	const QString HEX_DATA_ID("HexDataID");

	const QString APP_DATA_SERVICE_PROPERTY_IS_VALID1("AppDataServicePropertyIsValid1");
	const QString APP_DATA_SERVICE_ID1("AppDataServiceID1");
	const QString APP_DATA_SERVICE_IP1("AppDataServiceIP1");
	const QString APP_DATA_SERVICE_PORT1("AppDataServicePort1");
	const QString REALTIME_DATA_IP1("RealtimeDataIP1");
	const QString REALTIME_DATA_PORT1("RealtimeDataPort1");

	const QString APP_DATA_SERVICE_PROPERTY_IS_VALID2("AppDataServicePropertyIsValid2");
	const QString APP_DATA_SERVICE_ID2("AppDataServiceID2");
	const QString APP_DATA_SERVICE_IP2("AppDataServiceIP2");
	const QString APP_DATA_SERVICE_PORT2("AppDataServicePort2");
	const QString REALTIME_DATA_IP2("RealtimeDataIP2");
	const QString REALTIME_DATA_PORT2("RealtimeDataPort2");

	const QString TUNING_SERVICE_PROPERTY_IS_VALID("TuningServicePropertyIsValid");
	const QString TUNING_SERVICE_IP("TuningServiceIP");
	const QString TUNING_SERVICE_PORT("TuningServicePort");

	const QString SOFTWARE_METROLOGY_ID("SoftwareMetrologyID");
}

namespace Latin1Char
{
	const QLatin1Char ZERO('0');
	const QLatin1Char SPACE(' ');
}

namespace CfgFileId
{
	const QString APP_DATA_SOURCES("APP_DATA_SOURCES");
	const QString APP_SIGNALS("APP_SIGNALS");
	const QString APP_SIGNAL_SET("APP_SIGNAL_SET");
	const QString COMPARATOR_SET("COMPARATOR_SET");
	const QString UNIT_SET("UNIT_SET");

	const QString TUNING_SOURCES("TUNING_SOURCES");
	const QString TUNING_SIGNALS("TUNING_SIGNALS");
	const QString TUNING_SCHEMAS_DETAILS("TUNING_SCHEMAS_DETAILS");
	const QString TUNING_FILTERS("TUNING_FILTERS");
	const QString TUNING_GLOBALSCRIPT("TUNING_GLOBALSCRIPT");
	const QString TUNING_CONFIGARRIVEDSCRIPT("TUNING_CONFIGARRIVEDSCRIPT");

	const QString CLIENT_BEHAVIOR("CLIENT_BEHAVIOR");
	const QString LOGO("LOGO");

	const QString METROLOGY_ITEMS("METROLOGY_ITEMS");
	const QString METROLOGY_SIGNAL_SET("METROLOGY_SIGNAL_SET");
}

namespace Directory
{
	const QString COMMON("Common");

	const QString REPORTS("Reports");
	const QString OPTO_VHD("Opto-vhd");
	const QString RUN_SERVICE_SCRIPTS("RunServiceScripts");
	const QString BIN("Bin");
}

namespace File
{
	const QString APP_SIGNALS_ASGS("AppSignals.asgs");
	const QString COMPARATORS_SET("Comparators.set");
	const QString APP_DATA_SOURCES_XML("AppDataSources.xml");
	const QString TUNING_SOURCES_XML("TuningSources.xml");
	const QString CONFIGURATION_XML("Configuration.xml");
	const QString SOFTWARE_XML("Software.xml");
	const QString SUBSYSTEMS_XML("Subsystems.xml");

	const QString METROLOGY_ITEMS_XML("MetrologyItems.xml");
	const QString METROLOGY_SIGNAL_SET("MetrologySignal.set");

	const QString CONNECTIONS_TXT("Connections.txt");
	const QString CONNECTIONS_XML("Connections.xml");
	const QString LOGIC_MODULES_XML("LogicModules.xml");
}

namespace Separator
{
	const QString SEMICOLON_SPACE("; ");
	const QString SEMICOLON(";");
}



