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
	inline const QString RUP_TUNING_DATA_UID("RupTuningDataUID");
	inline const QString HEX_RUP_TUNING_DATA_UID("HexRupTuningDataUID");
	inline const QString FOTIP_TUNING_DATA_UID("FotipTuningDataUID");
	inline const QString HEX_FOTIP_TUNING_DATA_UID("HexFotipTuningDataUID");
	inline const QString TUNING_SOURCE_EQUIPMENT_ID("TuningSourceEquipmentID");

	inline const QString APP_DATA_ENABLE("AppDataEnable");
	inline const QString APP_DATA_IP("AppDataIP");
	inline const QString APP_DATA_PORT("AppDataPort");
	inline const QString APP_DATA_SERVICE_ID("AppDataServiceID");
	inline const QString APP_DATA_SERVICE_IDS("AppDataServiceIDs");
	inline const QString APP_DATA_SERVICE_IP("AppDataServiceIP");
	inline const QString APP_DATA_SERVICE_PORT("AppDataServicePort");
	inline const QString APP_DATA_SERVICE_NETMASK("AppDataServiceNetmask");
	inline const QString APP_DATA_SIZE_BYTES("AppDataSizeBytes");
	inline const QString RUP_APP_DATA_UID("RupAppDataUID");
	inline const QString HEX_RUP_APP_DATA_UID("HexRupAppDataUID");
	inline const QString APP_DATA_FRAMES_QUANTITY("AppDataFramesQuantity");
	inline const QString OVERRIDE_APP_DATA_WORD_COUNT("OverrideAppDataWordCount");

	inline const QString DIAG_DATA_ENABLE("DiagDataEnable");
	inline const QString DIAG_DATA_IP("DiagDataIP");
	inline const QString DIAG_DATA_PORT("DiagDataPort");
	inline const QString DIAG_DATA_SERVICE_ID("DiagDataServiceID");
	inline const QString DIAG_DATA_SERVICE_IDS("DiagDataServiceIDs");
	inline const QString DIAG_DATA_SERVICE_IP("DiagDataServiceIP");
	inline const QString DIAG_DATA_SERVICE_PORT("DiagDataServicePort");
	inline const QString DIAG_DATA_SERVICE_NETMASK("DiagDataServiceNetmask");
	inline const QString DIAG_DATA_SIZE_BYTES("DiagDataSizeBytes");
	inline const QString RUP_DIAG_DATA_UID("RupDiagDataUID");
	inline const QString HEX_RUP_DIAG_DATA_UID("HexRupDiagDataUID");
	inline const QString DIAG_DATA_FRAMES_QUANTITY("DiagDataFramesQuantity");
	inline const QString OVERRIDE_DIAG_DATA_WORD_COUNT("OverrideDiagDataWordCount");

	// LM properties
	//
	inline const QString APP_LAN_DATA_UID("AppLANDataUID");
	inline const QString APP_LAN_DATA_SIZE("AppLANDataSize");

	inline const QString DIAG_LAN_DATA_UID("DiagLANDataUID");
	inline const QString DIAG_LAN_DATA_SIZE("DiagLANDataSize");

	inline const QString TUNING_LAN_DATA_UID("TuningLANDataUID");

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

	inline const QString LM_PLATFORM_INTERFACE_CONTROLLER_SUFFIX = "_PI";

	inline const QString ACTUATOR_ID("ActuatorID");
	inline const QString ACTUATOR_DESCRIPTION("ActuatorDescription");

	inline const QString SC_FBLOCK_COUNT_SUFFIX("_SC_FBLOCK_COUNT");
	inline const QString SC_FSIM_COUNT_SUFFIX("_SC_FSIM_COUNT");
	inline const QString SC_FMISMATCH_COUNT_SUFFIX("_SC_FMISMATCH_COUNT");

	// I/O Modules properties
	//
	inline const QString TX_DATA_SIZE("TxDataSize");
	inline const QString TX_DIAG_DATA_OFFSET("TxDiagDataOffset");
	inline const QString TX_DIAG_DATA_SIZE("TxDiagDataSize");
	inline const QString TX_APP_DATA_OFFSET("TxAppDataOffset");
	inline const QString TX_APP_DATA_SIZE("TxAppDataSize");

	inline const QString RX_DATA_SIZE("RxDataSize");
	inline const QString RX_APP_DATA_OFFSET("RxAppDataOffset");
	inline const QString RX_APP_DATA_SIZE("RxAppDataSize");

	// CfgService properties
	//
	inline const QString CHECK_HOSTNAME("CheckHostname");

	// TuningService properties
	//
	inline const QString TUNING_DATA_NETMASK("TuningDataNetmask");
	inline const QString TUNING_DATA_IP("TuningDataIP");
	inline const QString TUNING_DATA_PORT("TuningDataPort");
	inline const QString SINGLE_LM_CONTROL("SingleLmControl");
	inline const QString DISABLE_MODULES_TYPE_CHECKING("DisableModulesTypeChecking");
	inline const QString ENABLE("Enable");
	inline const QString TUNING_SIM_IP("TuningSimIP");
	inline const QString TUNING_SIM_PORT("TuningSimPort");
	inline const QString CONTROLLER_SUFFIX_CH_TEMPLATE("_CH%1");

	// TuningClient properties
	//
	inline const QString APPLY_MODE("ApplyMode");
	inline const QString SHOW_SIGNALS("ShowSignals");
	inline const QString SHOW_SCHEMAS("ShowSchemas");
	inline const QString SCHEMAS_NAVIGATION("SchemasNavigation");
	inline const QString SHOW_SCHEMAS_LIST("ShowSchemasList");
	inline const QString SHOW_SCHEMAS_TABS("ShowSchemasTabs");
	inline const QString STATUS_FLAG_FUNCTION("StatusFlagFunction");
	inline const QString SHOW_SOR("ShowSOR");
	inline const QString USE_ACCESS_FLAG("UseAccessFlag");
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
	inline const QString DISCRETES_LOG_HOURS("DiscretesLogHours");
	inline const QString REQUEST_CONTROLLER_SUFFIX("_RC");

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

	// GatewayService propertiews
	//
	inline const QString GATEWAY_DESCRIPTION("GatewayDescription");

	// Properties used in several Services
	//
	inline const QString CLIENT_REQUEST_IP("ClientRequestIP");
	inline const QString CLIENT_REQUEST_NETMASK("ClientRequestNetmask");
	inline const QString CLIENT_REQUEST_PORT("ClientRequestPort");

	inline const QString SECURITY_LEVEL("SecurityLevel");

	inline const QString CFG_SERVICE_ID1("ConfigurationServiceID1");
	inline const QString CFG_SERVICE_IP1("ConfigurationServiceIP1");
	inline const QString CFG_SERVICE_PORT1("ConfigurationServicePort1");

	inline const QString CFG_SERVICE_ID2("ConfigurationServiceID2");
	inline const QString CFG_SERVICE_IP2("ConfigurationServiceIP2");
	inline const QString CFG_SERVICE_PORT2("ConfigurationServicePort2");

	inline const QString CFG_SERVICE_IDS("ConfigurationServiceIDs");

	inline const QString SOFTWARE_TYPE("SoftwareType");
	inline const QString HOSTNAME("Hostname");

	inline const QString APP_DATA_SERVICE_ID1("AppDataServiceID1");
	inline const QString APP_DATA_SERVICE_IP1("AppDataServiceIP1");
	inline const QString APP_DATA_SERVICE_PORT1("AppDataServicePort1");

	inline const QString APP_DATA_SERVICE_ID2("AppDataServiceID2");
	inline const QString APP_DATA_SERVICE_IP2("AppDataServiceIP2");
	inline const QString APP_DATA_SERVICE_PORT2("AppDataServicePort2");

	inline const QString START_SCHEMA_ID("StartSchemaID");
	inline const QString SCHEMA_TAGS("SchemaTags");

	inline const QString APP_SIGNAL_LIST_IDS("AppSignalListIDs");
	inline const QString APP_SIGNAL_LIST_MASKS("AppSignalListMasks");
	inline const QString APP_SIGNAL_LIST_TAGS("AppSignalListTags");
	
	inline const QString UI_CONFIGURATION("UiConfiguration");
	

	inline const QString FONTS("Fonts");
	inline const QString UNICODDE_SUBSETS("UnicodeSubsets");

	// DeviceAppSigal properties
	//
	inline const QString APP_SIGNAL_ID_TEMPLATE("AppSignalIDTemplate");
	inline const QString CUSTOM_APP_SIGNAL_ID_TEMPLATE("CustomAppSignalIDTemplate");
	inline const QString APP_SIGNAL_CAPTION_TEMPLATE("AppSignalCaptionTemplate");

	inline const QString ENABLE_TUNING("EnableTuning");
	inline const QString TUNING_DEFAULT_VALUE("TuningDefaultValue");
	inline const QString TUNING_LOW_BOUND("TuningLowBound");
	inline const QString TUNING_HIGH_BOUND("TuningHighBound");

	// Tuning Authorization properties
	//
	inline const QString TUNING_LOGIN("TuningLogin");
	inline const QString TUNING_USER_ACCOUNTS("TuningUserAccounts");
	inline const QString TUNING_SESSION_TIMEOUT("TuningSessionTimeout");
	inline const QString LOGIN_PER_OPERATION("LoginPerOperation");

	inline const QString TESTING_LOGIN("TestingLogin");
	inline const QString TESTING_USER_ACCOUNTS("TestingUserAccounts");

	// Testing Reports properties
	//

	inline const QString TESTING_SCRIPTTAGS("ScriptTags");
	inline const QString TESTING_PLANT("Plant");
	inline const QString TESTING_UNIT("Unit");
	inline const QString TESTING_SYSTEM("System");

	// Opto modules properties
	//
	inline const QString ALLOW_INCHASSIS_OPTO_CONNECTIONS("AllowInchassisOptoConnections");
}

namespace XmlElement
{
	inline const QString CONTENT("Content");
	inline const QString SOFTWARE_ITEMS("SoftwareItems");
	inline const QString SOFTWARE("Software");
	inline const QString SETTINGS("Settings");
	inline const QString CLIENTS("Clients");
	inline const QString CLIENT("Client");
	inline const QString BUILD_INFO("BuildInfo");

	inline const QString CFG_SERVICE1("CfgService1");
	inline const QString CFG_SERVICE2("CfgService2");

	inline const QString APP_DATA_SERVICE("AppDataService");
	inline const QString ARCHIVE_SERVICE("ArchiveService");

	inline const QString DIAG_DATA_SERVICE("DiagDataService");
	inline const QString TUNING_SERVICES("TuningServices");
	inline const QString TUNING_SERVICE("TuningService");

	inline const QString APPEARANCE("Appearance");

	inline const QString TUNING_CLIENTS("TuningClients");
	inline const QString TUNING_CLIENT("TuningClient");
	inline const QString TUNING_SOURCES("TuningSources");
	inline const QString TUNING_SOURCE("TuningSource");
	inline const QString TUNING_CHANNEL_TEMPLATE("Channel%1");
	inline const QString CHANNEL_COUNT("ChannelCount");

	inline const QString MATS_USERS("MatsUsers");
	inline const QString MATS_USER("MatsUser");

	inline const QString DATA_SOURCES("DataSources");
	inline const QString DATA_SOURCE("DataSource");
	inline const QString ASSOCIATED_SIGNALS("AssociatedSignals");

	inline const QString SETTINGS_SET("SettingsSet");

	inline const QString APP_DATA_SERVICES("AppDataServices");

	inline const QString SIGNAL_LOG("SignalLog");
	inline const QString SIGNAL_LOG_ATTRIBUTE_TAG_CRITICAL("TagCritical");
	inline const QString SIGNAL_LOG_ATTRIBUTE_TAG_WARNING("TagWarning");

	inline const QString TUNING_SECURITY("TuningSecurity");
	
	inline const QString TESTING_SETTINGS("TestingSettings");
	inline const QString TESTING_SECURITY("TestingSecurity");
	inline const QString TESTING_REPORTS("TestingReports");

	inline const QString LOGIC_MODULES("LogicModules");
	inline const QString LOGIC_MODULE("LogicModule");

	inline const QString LAN_CONTROLLERS("LanControllers");
	inline const QString LAN_CONTROLLER("LanController");

	inline const QString TUNING_PARAMS("TuningParams");
	inline const QString APP_DATA_PARAMS("AppDataParams");
	inline const QString DIAG_DATA_PARAMS("DiagDataParams");

	inline const QString APP_SIGNALS("AppSignals");
	inline const QString TUNING_SIGNALS("TuningSignals");
	inline const QString DIAG_SIGNALS("DiagSignals");
	inline const QString SW_CALC_SIGNALS("SwCalcSignals");

	inline const QString SIGNALS("Signals");
	inline const QString MODBUS_SIGNALS("ModbusSignals");
	inline const QString SIGNAL_ELEM("Signal");

	inline const QString ACTUATORS("Actuators");
	inline const QString ACTUATOR("Actuator");

	inline const QString GATEWAYS("Gateways");
	inline const QString GATEWAY("Gateway");
	inline const QString SIGNAL_LISTS("SignalLists");
	inline const QString SIGNAL_LIST("SignalList");
	inline const QString ID("ID");

	inline const QString DIAG_SIGNAL_TYPE("DiagSignalType");
	inline const QString DIAG_SIGNAL_TYPES("DiagSignalTypes");

	inline const QString TUNING_DATA("TuningData");
	inline const QString TUNING_FLASH_MEMORY("TuningFlashMemory");
	inline const QString TUNING_DATA_MEMORY("TuningDataMemory");
	inline const QString ANALOG_FLOAT_SIGNALS("AnalogFloatSignals");
	inline const QString ANALOG_INT32_SIGNALS("AnalogInt32Signals");
	inline const QString DISCRETE_SIGNALS("DiscreteSignals");

	inline const QString REQUEST_CONTROLLERS("RequestControllers");
	inline const QString REQUEST_CONTROLLER("RequestController");

	inline const QString BUILD_RESULT("BuildResult");
	inline const QString BUILD("Build");
	inline const QString FILES("Files");
	inline const QString FILE("File");
}

namespace XmlAttribute
{
	inline const QString ENUM_VALUE_SUFFIX("Val");

	inline const QString CAPTION("Caption");
	inline const QString COUNT("Count");
	inline const QString ID("ID");
	inline const QString IDs("IDs");
	inline const QString TYPE("Type");
	inline const QString BUILD_ID("BuildID");
	inline const QString CHANNEL("Channel");
	inline const QString APP_SIGNAL_ID("AppSignalID");
	inline const QString CUSTOM_APP_SIGNAL_ID("CustomAppSignalID");
	inline const QString EQUIPMENT_ID("EquipmentID");
	inline const QString DATA_FORMAT("DataFormat");
	inline const QString DATA_SIZE("DataSize");
	inline const QString SOFTWARE_CONTROLLERS("SoftwareControllers");
	inline const QString WORKSTATION_EQUIPMENT_ID("WorkstationEquipmentID");
	inline const QString LM_EQUIPMENT_ID("LmEquipmentID");
	inline const QString PROPS_MASK("PropsMask");

	inline const QString PROJECT("Project");
	inline const QString DATE("Date");
	inline const QString CHANGESET("Changeset");
	inline const QString USER("User");
	inline const QString WORKSTATION("Workstation");
	inline const QString NAME("Name");
	inline const QString TAG("Tag");
	inline const QString COMPRESSED("Compressed");
	inline const QString SIZE("Size");
	inline const QString MD5("MD5");
	inline const QString ERRORS("Errors");

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
	inline const QString CONTROLLER_EQUIPMENT_ID("ControllerEquipmentID");
	inline const QString DRIVEN_SOURCES("DrivenSources");

	inline const QString SOFTWARE_METROLOGY_ID("SoftwareMetrologyID");

	inline const QString MODULE_EQUIPMENT_ID("ModuleEquipmentID");
	inline const QString MODULE_PRESET_NAME("ModulePresetName");
	inline const QString LM_NUMBER("LmNumber");
	inline const QString SUBSYSTEM_CHANNEL("SubsystemChannel");
	inline const QString SUBSYSTEM_KEY("SubsystemKey");
	inline const QString SUBSYSTEM_ID("SubsystemID");
	inline const QString MODULE_TYPE("ModuleType");
	inline const QString MODULE_UNIQUE_ID("ModuleUniqueID");
	inline const QString MODULE_WORKCYCLE_MCS("ModuleWorkcycleMcs");

	inline const QString RUP_VERSION("RupVersion");
	inline const QString FOTIP_VERSION("FotipVersion");

	inline const QString GATEWAY_TYPE("GatewayType");
	inline const QString GATEWAY_ID("GatewayID");
	inline const QString GATEWAY_DESCRIPTION("GatewayDescription");
	inline const QString SIGNAL_LISTS_COUNT("SignalListsCount");

	inline const QString SYSTEM_ID("SystemID");
	inline const QString LOCAL_GATEWAY_IP1("LocalGatewayIP1");
	inline const QString REMOTE_GATEWAY_IP1("RemoteGatewayIP1");
	inline const QString LOCAL_GATEWAY_IP2("LocalGatewayIP2");
	inline const QString REMOTE_GATEWAY_IP2("RemoteGatewayIP2");
	inline const QString LISTS_VERSION("ListsVersion");
	inline const QString TIME_TYPE("TimeType");
	inline const QString PERIOD("Period");

	inline const QString LIST_NO("ListNo");
	inline const QString DATA_TYPE("DataType");
	inline const QString SEND_EVENTS("SendEvents");
	inline const QString INCLUDE_APP_SIGNAL_ID("IncludeAppSignalID");

	inline const QString SIGNAL_FORMAT("SignalFormat");
	inline const QString BYTE_ORDER_ATTR("ByteOrder");

	inline const QString SIGNAL_TYPE_ID("SignalTypeID");
	inline const QString SYSTEM_SIGNAL_TYPE("SystemSignalType");
	inline const QString DIAG_SIGNAL_TYPE("DiagSignalType");
	inline const QString DIAG_BYTE_ORDER("DiagByteOrder");
	inline const QString UNITS("Units");

	inline const QString INVERSE_VALUE("InverseValue");
	inline const QString NORMAL_STATE("NormalState");
	inline const QString NORMAL_STATE_STR0("NormalStateStr0");
	inline const QString NORMAL_STATE_STR1("NormalStateStr1");

	inline const QString DIAG_ANALOG_FORMAT("DiagAnalogFormat");
	inline const QString USE_LIMITS("UseLimits");
	inline const QString ADC_HIGH_LIMIT("AdcHighLimit");
	inline const QString ADC_LOW_LIMIT("AdcLowLimit");
	inline const QString VALUE_HIGH_LIMIT("ValueHighLimit");
	inline const QString VALUE_LOW_LIMIT("ValueLowLimit");
	inline const QString VALUE_MULTIPLIER("ValueMultiplier");
	inline const QString VALUE_OFFSET("ValueOffset");
	inline const QString UUID("Uuid");

	inline const QString LOGIN("Login");
	inline const QString ENABLED("Enabled");
	inline const QString DESCRIPTION("Description");
	inline const QString APP_SIGNAL_TAGS("AppSignalTags");
	inline const QString MATS_USERS("MatsUsers");

	inline const QString FRAME_COUNT("FrameCount");
	inline const QString FRAME_PAYLOAD_B("FramePayloadB");
	inline const QString FRAME_SIZE_B("FrameSizeB");

	inline const QString DATA_OFFSET_W("DataOffsetW");
	inline const QString DATA_SIZE_W("DataSizeW");
	inline const QString FRAME_PAYLOAD_W("FramePayloadW");
	inline const QString FRAME_SIZE_W("FrameSizeW");
	inline const QString USED_FRAMES_COUNT("UsedFramesCount");
	inline const QString SIGNALS_COUNT("SignalsCount");
	inline const QString FOTIP_TUNING_DATA_UID("FotipTuningDataUID");

	inline const QString MODBUS_DEVICE_ID("ModbusDeviceID");
	inline const QString MODBUS_MODE("ModbusMode");
	inline const QString REG_NO("RegNo");
	inline const QString REG_BIT("RegBit");
	inline const QString FORMAT("Format");
	inline const QString IS_CONST("IsConst");
	inline const QString CONST_VALUE("ConstValue");
	inline const QString UNIQ_SIGNALS_IN_ALL_LISTS("UniqSignalsInAllLists");
	inline const QString UNIQ_SIGNALS_IN_LIST("UniqSignalsInList");

	inline const QString MODULE_CAPTION("ModuleCaption");

	inline const QString CLIENT_REQUEST_IP("ClientRequestIP");
	inline const QString CLIENT_REQUEST_IP1("ClientRequestIP1");
	inline const QString CLIENT_REQUEST_IP2("ClientRequestIP2");
	inline const QString CLIENT_REQUEST_NETMASK("ClientRequestNetmask");
	inline const QString RT_TRENDS_REQUEST_IP("RtTrendsRequestIP");
	inline const QString SECURITY_LEVEL("SecurityLevel");
	inline const QString ENABLE("Enable");
}

namespace Latin1Char
{
	inline const QLatin1Char ZERO('0');
	inline const QLatin1Char SPACE(' ');
}

namespace CfgFileId
{
	inline const QString CONFIGURATION_XML("CONFIGURATION_XML");

	inline const QString APP_DATA_SOURCES("APP_DATA_SOURCES");
	inline const QString APP_SIGNALS("APP_SIGNALS");
	inline const QString ACQUIRED_APP_SIGNALS("ACQUIRED_APP_SIGNALS");
	inline const QString APP_SIGNALS_EXT("APP_SIGNALS_EXT");
	inline const QString APP_SIGNAL_SET("APP_SIGNAL_SET");
	inline const QString APP_SIGNALS_XML("APP_SIGNALS_XML");
	inline const QString COMPARATOR_SET("COMPARATOR_SET");
	inline const QString UNIT_SET("UNIT_SET");

	inline const QString DIAG_DATA_SOURCES("DIAG_DATA_SOURCES");
	inline const QString DIAG_SIGNAL_TYPES("DIAG_SIGNAL_TYPES");
	inline const QString ACQUIRED_DIAG_SIGNALS("ACQUIRED_DIAG_SIGNALS");

	inline const QString TUNING_SOURCES("TUNING_SOURCES");
	inline const QString TUNING_SIGNALS("TUNING_SIGNALS");
	inline const QString TUNING_SCHEMAS_DETAILS("TUNING_SCHEMAS_DETAILS");
	inline const QString TUNING_UI("TUNING_UI");
	inline const QString TUNING_GLOBALSCRIPT("TUNING_GLOBALSCRIPT");

	inline const QString CLIENT_BEHAVIOR("CLIENT_BEHAVIOR");
	inline const QString MONITOR_EQUIPMENT("MONITOR_EQUIPMENT");
	inline const QString MATSUSERS("MATSUSERS_XML");
	inline const QString LOGO("LOGO");

	inline const QString METROLOGY_ITEMS("METROLOGY_ITEMS");
	inline const QString METROLOGY_SIGNAL_SET("METROLOGY_SIGNAL_SET");

	inline const QString TESTSUITE_TESTSCRIPT("TESTSUITE_TESTSCRIPT");
	inline const QString TESTSUITE_REPORTTEMPLATES("TESTSUITE_REPORTTEMPLATES");

	inline const QString GATEWAY_DESCRIPTION("GATEWAY_DESCRIPTION");
}

namespace CfgFileTag
{
	inline const QString APPSIGNALLISTS("APPSIGNALLISTS");
}

namespace  SettingsProfile
{
	inline const QString DEFAULT("Default");
}

namespace SoftwareSetting
{
	inline const QString EQUIPMENT_ID("EquipmentID");

	inline const QString CFG_SERVICE_IP1("CfgServiceIP1");
	inline const QString CFG_SERVICE_IP2("CfgServiceIP2");

	inline const QString AUTOLOAD_BUILD_PATH("AutoloadBuildPath");
	inline const QString CLIENT_REQUEST_IP("ClientRequestIP");
	inline const QString WORK_DIRECTORY("WorkDirectory");
	inline const QString CHECK_HOSTNAME("CheckHostname");
	inline const QString CURRENT_PROFILE("CurrentSoftwareSettingsProfile");

	inline const QString RUN_MODE("RunMode");
	inline const QString SIMULATION("simulation");

	inline const QString ARCHIVE_LOCATION("ArchiveLocation");
	inline const QString MIN_QUEUE_SIZE_FOR_FLUSHING("MinQueueSizeForFlushing");

	inline const QString PROCESSING_THREADS_COUNT("ProcessingThreadsCount");
	inline const QString OVERRIDE_APP_DATA_RECEIVING_IP("OverrideAppDataReceivingIP");
	inline const QString OVERRIDE_DIAG_DATA_RECEIVING_IP("OverrideDiagDataReceivingIP");

	inline const QString GATEWAY_DESCRIPTION_FILE("GatewayDescriptionFile");
	inline const QString LOG_GATEWAY_PACKETS("LogGatewayPackets");

	inline const QString LOG_RUP_TIME_ERRORS("LogRupTimeErrors");

	inline const QString READ_ONLY_ARCHIVE_PATH("ReadOnlyArchivePath");
}

namespace CmdLineArg
{
	inline const QString HELP("h");
	inline const QString VERSION("v");
	inline const QString EXEC_AS_APP("e");
	inline const QString INSTALL("i");
	inline const QString UNINSTALL("u");
	inline const QString TERMINATE("t");
	inline const QString INSTANCE("inst");
	inline const QString CLEAR("clr");

	inline const QString ID("id");
	inline const QString IP("ip");
	inline const QString CFG_IP1("cfgip1");
	inline const QString CFG_IP2("cfgip2");

	inline const QString CFG_FILE("f");
	inline const QString CFG_PARSE("parse");

	inline const QString CHECKHOSTNAME("checkhostname");
	inline const QString PROFILE("profile");
	inline const QString MODE("mode");
	inline const QString WORK_DIRECTORY("w");
	inline const QString BUILD_PATH("b");

	inline const QString LOG_GW("logGw");
	inline const QString LOG_RUP_TIME_ERR("logRupTimeErr");

	inline const QString READ_ONLY("readonly");
	inline const QString CLIENT_IP("clientip");

	inline const QString PTC("ptc");
	inline const QString RECVIP("recvip");
}

namespace Directory
{
	inline const QString COMMON("Common");

	inline const QString REPORTS("Reports");
	inline const QString OPTO_VHD("Opto-vhd");

	inline const QString RUN_SERVICE_SCRIPTS_WINDOWS("RunServiceScripts/Windows");
	inline const QString RUN_SERVICE_SCRIPTS_LINUX("RunServiceScripts/Linux");

	inline const QString BIN("Bin");
	inline const QString SUBSYSTEMS("Subsystems");
	inline const QString TESTS("Tests");
	inline const QString APP_SIGNAL_LISTS("AppSignalLists");
}

namespace File
{
	inline const QString APP_SIGNALS_ASGS("AppSignals.asgs");
	inline const QString ACQUIRED_APP_SIGNALS_ASGS("AcquiredAppSignals.asgs");
	inline const QString APP_SIGNALS_XML("AppSignals.xml");
	inline const QString APP_SIGNALS_EXT_XML("AppSignalsExt.xml");
	inline const QString APP_SIGNALS_LIST_CSV("AppSignalsList.csv");
	inline const QString COMPARATORS_SET("Comparators.set");
	inline const QString APP_DATA_SOURCES_XML("AppDataSources.xml");
	inline const QString TUNING_SOURCES_XML("TuningSources.xml");
	inline const QString CONFIGURATION_XML("Configuration.xml");
	inline const QString SOFTWARE_XML("Software.xml");
	inline const QString SUBSYSTEMS_XML("Subsystems.xml");
	inline const QString MATSUSERS_XML("MatsUsers.xml");

	inline const QString DIAG_SIGNAL_TYPES_XML("DiagSignalTypes.xml");
	inline const QString DIAG_DATA_SOURCES_XML("DiagDataSources.xml");
	inline const QString ACQUIRED_DIAG_SIGNALS_ASGS("AcquiredDiagSignals.asgs");
	inline const QString MONITOR_EQUIPMENT("MonitorEquipment.dat");

	inline const QString METROLOGY_ITEMS_XML("MetrologyItems.xml");
	inline const QString METROLOGY_SIGNAL_SET("MetrologySignal.set");
	inline const QString METROLOGY_CONNECTIONS_CSV = "MetrologyConnections.csv";

	inline const QString CONNECTIONS_TXT("Connections.txt");
	inline const QString CONNECTIONS_XML("Connections.xml");
	inline const QString LOGIC_MODULES_XML("LogicModules.xml");

	inline const QString SIM_PROFILES = "SimProfiles.txt";
	inline const QString RESOURCES_TXT = "Resources.txt";

	inline const QString GATEWAY_DESCRIPTION_TXT = "GatewayDescription.txt";
	inline const QString GATEWAY_DESCRIPTION_XML = "GatewayDescription.xml";

	inline const QString CRYPTO_SS_SERVER_CERTIFICATE = "/Crypto/ss_server.crt";
	inline const QString CRYPTO_SS_SERVER_PRIVATE_KEY = "/Crypto/ss_server_private.key";

	inline const QString CRYPTO_CA_SERVER_CERTIFICATE = "/Crypto/ca_server.crt";
	inline const QString CRYPTO_CA_SERVER_PRIVATE_KEY = "/Crypto/ca_server_private.key";

	inline const QString CRYPTO_CA_CLIENT_CERTIFICATE = "/Crypto/ca_client.crt";
	inline const QString CRYPTO_CA_CLIENT_PRIVATE_KEY = "/Crypto/ca_client_private.key";

	inline const QString ARCH_INFO_PROTO = "ArchInfo.proto";
	inline const QString ARCH_INFO_PROTO_BAK = "ArchInfo.proto.bak";
	inline const QString ARCHIVE_INFO = "archive.info";

	inline const QString READONLY = ".readonly";

	inline const QString MONITOR_BEHAVIOR = "MonitorBehavior.xml";

	inline const QString GLOBAL_SCRIPT = "GlobalScript.js";
	inline const QString GLOBAL_SCRIPT_FULL_PATH = "$root$/Tests/GlobalScript.js";

	inline static const QString VDU_APP_SIGNALS_BIN = "VduAppSignals.bin";
	inline static const QString VDU_APP_SIGNALS_TXT = "VduAppSignals.txt";

	inline static const QString VDU_GLOBAL_SCRIPT_LUA = "GlobalScript.lua";
	inline static const QString VDU_GLOBAL_SCRIPT_LBC = "GlobalScript.lbc"; // Lua Byte Code

	inline static const QString VDU_STARTUP_LOGO = "StartupLogo.bmp";

	inline static const QString SUBSYSTEM_DESC_JSON = "Description.json";

	// Moved from DbStruct
	//
	inline const QString SignalPropertyBehaviorFileName = "SignalPropertyBehavior.csv";
	inline const QString TagsFileName = "Tags.csv";
	inline const QString SimProfilesFileName = "SimProfiles.txt";

	inline const QString AlFileExtension = "als";                  // Application Logic schema file extension
	inline const QString AlTemplExtension = "templ_als";           // Application Logic schema template file extension

	inline const QString UfbFileExtension = "ufb";                 // User Functional Block schema file extension
	inline const QString UfbTemplExtension = "templ_ufb";          // User Functional Block template file extension

	inline const QString MvsFileExtension = "mvs";                 // Monitor schema file extension
	inline const QString MvsTemplExtension = "templ_mvs";          // Monitor schema template file extension

	inline const QString TvsFileExtension = "tvs";                 // TuningClient schema file extension
	inline const QString TvsTemplExtension = "templ_tvs";          // TuningClient schema template file extension

	inline const QString DvsFileExtension = "dvs";                 // Diagnostics schema file extension
	inline const QString DvsTemplExtension = "templ_dvs";          // Diagnostics schema template file extension

	inline const QString VduFileExtension = "vus";                 // VDU schema file extension
	inline const QString VduTemplExtension = "templ_vus";          // VDU schema template file extension
	inline const QString VduNativeFileExtension = "vbs";           // VDU schema file extension in native VDU format

	inline const QString OclFileExtension = "ocl";                 // (Optical) Connection Link
	inline const QString BusFileExtension = "bus_type";            // Bus type

	inline const QString AppSignalFileExtension = "asg";           // Application signal file extention (::Proto::AppSignal message)
	inline const QString AppSignalSetFileExtension = "asgs";       // AppSignalSet file extention (::Proto::AppSignalSet message)

	inline const QString AppSignalListFileExtension = "aslist";    // Application signals list file extention

	inline const QString JavaScriptFileExtension = "js";           // JavaScript file extension

	inline const QString DiagSignalTypeFileExtension = "dsgt";     // Diagnostics Signal Type file extension
	inline const QString DiagSignalTypeSetFileExtension = "dsgts"; // Diagnostics Signal Types set file extension
} // namespace File

namespace Separator
{
	inline const QString SEMICOLON_SPACE("; ");
	inline const QString SEMICOLON(";");
	inline const QString MINUS("-");
	inline const QString DOT(".");
	inline const QString COMMA(",");
	inline const QString SPACE(" ");
	inline const QString COMMA_SPACE(", ");
	inline const QString LINE("-------------------------------------------------------------------------------");
	inline const QString DIR("/");
	inline const QString BACK_DIR("\\");
	inline const QString EMPTY_STR("");
	inline const QString NEW_LINE("\n");
	inline const QString CR("\r");					// Carriage Return
	inline const QString LF("\n");					// Line Feed
	inline const QString TAB("\t");					// Line Feed
	inline const QString CR_LF("\r\n");
	inline const QString UNDERSCORE("_");
	inline const QString DOUBLE_QUOTES("\"");
	inline const QString YES("Yes");
	inline const QString NO("No");
	inline const QString QUESTIONS("???");
}

namespace FormatStr
{
	inline const QString POSTGRES_DATE_TIME("yyyy-MM-ddTHH:mm:ss");
	inline const QString DATE_TIME_FORMAT_STR("%1:%2:%3.%4 %5.%6.%7");
}

namespace TemplateMacro
{
	inline const QString START_TOKEN("$(");
	inline const QString END_TOKEN(")");
}

namespace AppSignalPropNames
{
	inline const QString ID("ID");										// Optimization, to share one string among all Signal instances
	inline const QString SIGNAL_GROUP_ID("SignalGroupID");
	inline const QString SIGNAL_INSTANCE_ID("SignalInstanceID");
	inline const QString CHANGESET_ID("ChangesetID");
	inline const QString CHECKED_OUT("CheckedOut");
	inline const QString USER_ID("UserID");
	inline const QString CHECKOUT_BY_USER("CheckoutByUser");
	inline const QString CHANNEL("Channel");
	inline const QString EXCLUDE_FROM_BUILD("ExcludeFromBuild");
	inline const QString INVERT_SIGNAL("InvertSignal");
	inline const QString CREATED("Created");
	inline const QString DELETED("Deleted");
	inline const QString INSTANCE_CREATED("InstanceCreated");
	inline const QString TYPE("Type");
	inline const QString IN_OUT_TYPE("InOutType");
	inline const QString SOFTWARE_CALC_FUNCTION("SoftwareCalcFunction");
	inline const QString APP_SIGNAL_ID("AppSignalID");
	inline const QString CUSTOM_APP_SIGNAL_ID("CustomAppSignalID");
	inline const QString BUS_TYPE_ID("BusTypeID");
	inline const QString CAPTION("Caption");
	inline const QString ANALOG_SIGNAL_FORMAT("AnalogSignalFormat");
	inline const QString DATA_SIZE("DataSize");
	inline const QString LOW_ADC("LowADC");
	inline const QString HIGH_ADC("HighADC");
	inline const QString LOW_DAC("LowDAC");
	inline const QString HIGH_DAC("HighDAC");
	inline const QString LOW_ENGINEERING_UNITS("LowEngineeringUnits");
	inline const QString HIGH_ENGINEERING_UNITS("HighEngineeringUnits");
	inline const QString LOW_PHYSICAL_UNITS("LowPhysicalUnits");
	inline const QString HIGH_PHYSICAL_UNITS("HighPhysicalUnits");
	inline const QString UNIT("Unit");
	inline const QString LOW_VALID_RANGE("LowValidRange");
	inline const QString HIGH_VALID_RANGE("HighValidRange");
	inline const QString ELECTRIC_LOW_LIMIT("ElectricLowLimit");
	inline const QString ELECTRIC_HIGH_LIMIT("ElectricHighLimit");
	inline const QString ELECTRIC_UNIT("ElectricUnit");
	inline const QString ELECTRIC_UNIT_STR("ElectricUnitStr");
	inline const QString RLOAD_OHM("Rload_Ohm");
	inline const QString SENSOR_TYPE("SensorType");
	inline const QString SENSOR_TYPE_STR("SensorTypeStr");
	inline const QString R0_OHM("R0_Ohm");
	inline const QString OUTPUT_MODE("OutputMode");
	inline const QString OUTPUT_MODE_STR("OutputModeStr");
    inline const QString INPUT_RANGE("InputRange");
	inline const QString ACQUIRE("Acquire");
	inline const QString ARCHIVE("Archive");
	inline const QString LOG("Log");
	inline const QString RESERVED("Reserved");
	inline const QString DECIMAL_PLACES("DecimalPlaces");
	inline const QString COARSE_APERTURE("CoarseAperture");
	inline const QString FINE_APERTURE("FineAperture");
	inline const QString OBSOLETE_ADAPTIVE_APERTURE("AdaptiveAperture");
	inline const QString APERTURE_TYPE("ApertureType");
	inline const QString FILTERING_TIME("FilteringTime");
	inline const QString SPREAD_TOLERANCE("SpreadTolerance");
	inline const QString BYTE_ORDER_PROP("ByteOrder");
	inline const QString EQUIPMENT_ID("EquipmentID");

	inline const QString ENABLE_TUNING("EnableTuning");
	inline const QString TUNING_VALUE_TYPE("TuningValueType");
	inline const QString TUNING_VALUE_TYPE_STR("TuningValueTypeStr");
	inline const QString TUNING_DEFAULT_VALUE("TuningDefaultValue");
	inline const QString TUNING_LOW_BOUND("TuningLowBound");
	inline const QString TUNING_HIGH_BOUND("TuningHighBound");

	inline const QString SPEC_PROP_STRUCT("SpecPropStruct");
	inline const QString SPECIFIC_PROPERTIES_STRUCT("SpecificPropertiesStruct");
	inline const QString PROTO_SPEC_PROP_VALUES("ProtoSpecPropValues");
	inline const QString TAGS("Tags");
	inline const QString UAL_ADDR("UalAddr");
	inline const QString REG_VALUE_ADDR("RegValueAddr");
	inline const QString REG_VALIDITY_ADDR("RegValidityAddr");
	inline const QString TUNING_ADDR("TuningAddr");
	inline const QString TUNING_ABS_ADDR("TuningAbsAddr");

	inline const QString MISPRINT_lowEngineeringUnitsCaption("LowEngeneeringUnits");
	inline const QString MISPRINT_highEngineeringUnitsCaption("HighEngeneeringUnits");
}

namespace AppSignalTags
{
	inline const QString in = QStringLiteral("in");
	inline const QString out = QStringLiteral("out");
	inline const QString view_linear = QStringLiteral("view_linear");
	inline const QString view_log10 = QStringLiteral("view_log10");
	inline const QString view_period = QStringLiteral("view_period");
} // namespace AppSignalTags

namespace AppSignalDefaultSpecPropStruct
{
	inline const QString INPUT_ANALOG(
		"4;ElectricHighLimit;5 Electric parameters;double;;;0;10;false;false;Electric high limit of input signal;true;None\n"
		"4;ElectricLowLimit;5 Electric parameters;double;;;0;10;false;false;Electric low limit of input signal;true;None\n"
		"4;ElectricUnit;5 Electric parameters;DynamicEnum [NoUnit=0,mA=1,mV=2,Ohm=3,V=4];;;NoUnit;0;false;false;;true;None\n"
		"4;FilteringTime;4 Signal processing;double;;;0.005;5;false;false;Signal filtering time in seconds;true;None\n"
		"4;HighADC;4 Signal processing;uint32;0;65535;65535;0;false;false;High ADC value;true;None\n"
		"4;HighEngineeringUnits;4 Signal processing;double;;;100;10;false;false;High engineering units;true;None\n"
		"4;HighValidRange;4 Signal processing;double;;;100;10;false;false;High valid range of signal;true;None\n"
		"4;LowADC;4 Signal processing;uint32;0;65535;0;0;false;false;Low ADC value;true;None\n"
		"4;LowEngineeringUnits;4 Signal processing;double;;;0;10;false;false;Low engineering units;true;None\n"
		"4;LowValidRange;4 Signal processing;double;;;0;10;false;false;Low valid range of signal;true;None\n"
		"4;SensorType;5 Electric parameters;DynamicEnum [NoSensor=0,Ohm_Pt50_W1391=1,Ohm_Pt100_W1391=2,Ohm_Pt50_W1385=3,Ohm_Pt100_W1385=4,Ohm_Cu_50_W1428=5,Ohm_Cu_100_W1428=6,Ohm_Cu_50_W1426=7,Ohm_Cu_100_W1426=8,Ohm_Pt21=9,Ohm_Cu23=10,mV_K_TXA=11,mV_L_TXK=12,mV_N_THH=13];;;NoSensor;0;false;false;;true;None\n"
		"4;SpreadTolerance;4 Signal processing;double;;;2;5;false;false;Spread tolerance of signal measurement channels in percents;true;None");

	inline const QString OUTPUT_ANALOG(
		"4;ElectricHighLimit;5 Electric parameters;double;;;0;10;false;false;Electric high limit of input signal;true;None\n"
		"4;ElectricLowLimit;5 Electric parameters;double;;;0;10;false;false;Electric low limit of input signal;true;None\n"
		"4;ElectricUnit;5 Electric parameters;DynamicEnum [NoUnit=0,mA=1,mV=2,Ohm=3,V=4];;;NoUnit;0;false;false;;true;None\n"
		"4;HighDAC;4 Signal processing;uint32;0;65535;65535;0;false;false;High DAC value;true;None\n"
		"4;HighEngineeringUnits;4 Signal processing;double;;;100;10;false;false;High engineering units;true;None\n"
		"4;LowDAC;4 Signal processing;uint32;0;65535;0;0;false;false;Low DAC value;true;None\n"
		"4;LowEngineeringUnits;4 Signal processing;double;;;0;10;false;false;Low engineering units;true;None\n"
		"4;OutputMode;5 Electric parameters;DynamicEnum [Plus0_Plus5_V=0,Plus4_Plus20_mA=1,Minus10_Plus10_V=2,Plus0_Plus5_mA=3];;;Plus0_Plus5_V;0;false;false;;true;None\n");

	inline const QString INTERNAL_ANALOG(
		"4;HighEngineeringUnits;4 Signal processing;double;;;100;10;false;false;High engineering units;true;None\n"
		"4;LowEngineeringUnits;4 Signal processing;double;;;0;10;false;false;Low engineering units;true;None\n");

	inline const QString BUS_CHILD_ANALOG(
		"4;HighADC;4 Signal processing;uint32;0;65535;65535;0;false;false;High ADC value;true;None\n"
		"4;HighEngineeringUnits;4 Signal processing;double;;;100;10;false;false;High engineering units;true;None\n"
		"4;HighValidRange;4 Signal processing;double;;;100;10;false;false;High valid range of signal;true;None\n"
		"4;LowADC;4 Signal processing;uint32;0;65535;0;0;false;false;Low ADC value;true;None\n"
		"4;LowEngineeringUnits;4 Signal processing;double;;;0;10;false;false;Low engineering units;true;None\n"
		"4;LowValidRange;4 Signal processing;double;;;0;10;false;false;Low valid range of signal;true;None\n");
}

namespace Manufacturer
{
	inline const QString RADIY("RadiyQt6");		// Radiy -> RadiyQt6, thus settings can be stored (for sw with Qt5 and Qt6)
												// in different registry locations as Qt5 and Qt6 have distinct settings format (for UI)
	inline const QString RADIY_ORGANIZATION("Radiy");
	inline const QString SITE("radiy.com");

	inline const QString CONFIGURATION_SERVICE("Configuration Service");
	inline const QString APPLICATION_DATA_SERVICE("Application Data Service");
	inline const QString DIAGNOSTIC_DATA_SERVICE("Diagnostic Data Service");
	inline const QString ARCHIVING_SERVICE("Archiving Service");
	inline const QString TUNING_SERVICE("Tuning Service");
	inline const QString GATEWAY_SERVICE("Gateway Service");
	inline const QString SERVICE_CONTROL_MANAGER("Service Control Manager");
	inline const QString METROLOGY("Metrology");
}

namespace Busses
{
	inline const QString SIGNAL_ID_SEPARATOR(".");

	inline const QString MACRO_BUS_TYPE("$(BusType)");
	inline const QString MACRO_BUS_APP_SIGNAL_ID("$(BusAppSignalID)");
	inline const QString MACRO_BUS_CUSTOM_APP_SIGNAL_ID("$(BusCustomAppSignalID)");
	inline const QString MACRO_BUS_CAPTION("$(BusCaption)");
}

namespace Afb
{
	//

	inline const QString SCALE("SCALE");

	inline const QString SCALE_SI32_SI32("scale_si_si");
	inline const QString SCALE_SI32_FP32("scale_si_fp");
	inline const QString SCALE_SI32_UI16("scale_si_16ui");

	inline const QString SCALE_FP32_SI32("scale_fp_si");
	inline const QString SCALE_FP32_FP32("scale_fp_fp");
	inline const QString SCALE_FP32_UI16("scale_fp_16ui");

	inline const QString SCALE_UI16_SI32("scale_16ui_si");
	inline const QString SCALE_UI16_FP32("scale_16ui_fp");
	inline const QString SCALE_UI16_UI16("scale_16ui_16ui");

	inline const QString SCALE_PARAM_X1("X1");
	inline const QString SCALE_PARAM_X2("X2");
	inline const QString SCALE_PARAM_Y1("Y1");
	inline const QString SCALE_PARAM_Y2("Y2");

	//

	inline const QString TCONV_SI32_FP32("tconv_si_fp");
	inline const QString TCONV_FP32_SI32("tconv_fp_si");

	inline const QString TCONV_BO_16("tconv_bo_16");
	inline const QString TCONV_BO_32("tconv_bo_32");

	inline const QString TCONV_SI16_SI32("tconv_si16_si32");
	inline const QString TCONV_UI16_SI32("tconv_ui16_si32");

	//

	inline const QString SWITCH_SI("switch_si");

	inline const QString SWITCH_SI_PIN_SELECT("sel");
	inline const QString SWITCH_SI_PIN_X1("in_x1");
	inline const QString SWITCH_SI_PIN_X2("in_x2");
	inline const QString SWITCH_SI_PIN_OUTPUT("out");

	// Software generated type conversions
	//
	inline const QString SW_TCONV_SI32_SI16("sw_tconv_si32_si16");
	inline const QString SW_TCONV_SI32_UI16("sw_tconv_si32_ui16");

	inline const QString SW_TCONV_SI16_SI32("sw_tconv_si16_si32");
	inline const QString SW_TCONV_UI16_SI32("sw_tconv_ui16_si32");

	inline const QString SW_TCONV_FP32_SI16("sw_tconv_fp32_si16");
	inline const QString SW_TCONV_FP32_UI16("sw_tconv_fp32_ui16");

	inline const QString SW_TCONV_SI16_FP32("sw_tconv_si16_fp32");
	inline const QString SW_TCONV_UI16_FP32("sw_tconv_ui16_fp32");

	//

	inline const QString NO_AFB("");
	inline const QString OR("|");
	inline const QString NEXT(",");

	//

	inline const QString IN_PIN_CAPTION("in");
	inline const QString OUT_PIN_CAPTION("out");

	inline const QString IN_1_PIN_CAPTION("in_1");
	inline const QString IN_2_PIN_CAPTION("in_2");
	inline const QString IN_3_PIN_CAPTION("in_3");
	inline const QString IN_4_PIN_CAPTION("in_4");

	inline const QString OUT_1_PIN_CAPTION("out_1");
	inline const QString OUT_2_PIN_CAPTION("out_2");
	inline const QString OUT_3_PIN_CAPTION("out_3");
	inline const QString OUT_4_PIN_CAPTION("out_4");

	inline const QString SIMLOCK_SIM_PIN_CAPTION("sim");
	inline const QString SIMLOCK_BLOCK_PIN_CAPTION("block");

	inline const QString VALIDITY_PIN_CAPTION("validity");
	inline const QString SIMULATED_PIN_CAPTION("simulated");
	inline const QString BLOCKED_PIN_CAPTION("blocked");
	inline const QString MISMATCH_PIN_CAPTION("mismatch");
	inline const QString HIGH_LIMIT_PIN_CAPTION("high_limit");
	inline const QString LOW_LIMIT_PIN_CAPTION("low_limit");

	inline const QString PARAM_I_CONF("i_conf");
	inline const QString PARAM_I_DATA_X1("i_data_x1");
	inline const QString PARAM_I_DATA_X2("i_data_x2");

	inline const QString PARAM_I_OPRD_QUANT("i_oprd_quant");
	inline const QString PARAM_I_BUS_WIDTH("i_bus_width");

	inline const QString PIN_I_1_OPRD("i_1_oprd");
	inline const QString PIN_O_RESULT("o_result");

	inline const QString AFB_NOT("not");
	inline const QString AFB_BUS_NOT("bus_not");
	inline const QString AFB_OR("or");
	inline const QString AFB_AND("and");

	inline const QString SET_FLAGS("set_flags");
	inline const QString SIMLOCK("simlock");
	inline const QString MISMATCH("mismatch");
}

namespace LmDescriptionName
{
	inline const QString LM1_SR04("LM1_SR04");
	inline const QString LM1_SR05("LM1_SR05");
	inline const QString LM1_SR20("LM1_SR20");

	inline const QString LM8_SR10("LM8_SR10");

	inline const QString LM11_SR90("LM11_SR90");
}

namespace BOM
{
	inline const QByteArray UTF8("\xEF\xBB\xBF");
}

