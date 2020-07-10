#pragma once

namespace EquipmentPropNames
{
	// Ethernet controller properties
	//
	const QString LAN_CONTROLLER_TYPE = "LanControllerType";
	const QString CONTROLLER_NO = "ControllerNo";				// equals to Place

	const QString TUNING_ENABLE = "TuningEnable";
	const QString TUNING_IP = "TuningIP";
	const QString TUNING_PORT = "TuningPort";
	const QString TUNING_SERVICE_ID = "TuningServiceID";
	const QString TUNING_SERVICE_IP = "TuningServiceIP";
	const QString TUNING_SERVICE_PORT = "TuningServicePort";
	const QString TUNING_SERVICE_NETMASK = "TuningServiceNetmask";
	const QString TUNING_SOURCE_EQUIPMENT_ID = "TuningSourceEquipmentID";

	const QString APP_DATA_ENABLE = "AppDataEnable";
	const QString APP_DATA_IP = "AppDataIP";
	const QString APP_DATA_PORT = "AppDataPort";
	const QString APP_DATA_SERVICE_ID = "AppDataServiceID";
	const QString APP_DATA_SERVICE_IP = "AppDataServiceIP";
	const QString APP_DATA_SERVICE_PORT = "AppDataServicePort";
	const QString APP_DATA_SERVICE_NETMASK = "AppDataServiceNetmask";
	const QString APP_DATA_SIZE_BYTES = "AppDataSizeBytes";
	const QString APP_DATA_UID = "AppDataUID";
	const QString HEX_APP_DATA_UID = "HexAppDataUID";
	const QString APP_DATA_FRAMES_QUANTITY = "AppDataFramesQuantity";
	const QString OVERRIDE_APP_DATA_WORD_COUNT = "OverrideAppDataWordCount";

	const QString DIAG_DATA_ENABLE = "DiagDataEnable";
	const QString DIAG_DATA_IP = "DiagDataIP";
	const QString DIAG_DATA_PORT = "DiagDataPort";
	const QString DIAG_DATA_SERVICE_ID = "DiagDataServiceID";
	const QString DIAG_DATA_SERVICE_IP = "DiagDataServiceIP";
	const QString DIAG_DATA_SERVICE_PORT = "DiagDataServicePort";
	const QString DIAG_DATA_SERVICE_NETMASK = "DiagDataServiceNetmask";
	const QString DIAG_DATA_SIZE_BYTES = "DiagDataSizeBytes";
	const QString DIAG_DATA_UID = "DiagDataUID";
	const QString HEX_DIAG_DATA_UID = "HexDiagDataUID";
	const QString DIAG_DATA_FRAMES_QUANTITY = "DiagDataFramesQuantity";
	const QString OVERRIDE_DIAG_DATA_WORD_COUNT = "OverrideDiagDataWordCount";


	// LM properties
	//
	const QString LM_APP_LAN_DATA_UID = "AppLANDataUID";
	const QString LM_APP_LAN_DATA_SIZE = "AppLANDataSize";

	const QString LM_DIAG_LAN_DATA_UID = "DiagLANDataUID";
	const QString LM_DIAG_LAN_DATA_SIZE = "DiagLANDataSize";

	const QString EQUIPMENT_ID = "EquipmentID";
	const QString CAPTION = "Caption";

	const QString SUBSYSTEM_ID = "SubsystemID";
	const QString LM_NUMBER = "LMNumber";
	const QString SUBSYSTEM_CHANNEL = "SubsystemChannel";

	const QString MODULE_FAMILY = "ModuleFamily";
	const QString MODULE_FAMILY_ID = "ModuleFamilyID";
	const QString MODULE_VERSION = "ModuleVersion";

	const QString PRESET_NAME = "PresetName";
	const QString LM_DESCRIPTION_FILE = "LmDescriptionFile";

	// TuningService properties
	//
	const QString TUNING_DATA_NETMASK = "TuningDataNetmask";
	const QString TUNING_DATA_IP = "TuningDataIP";
	const QString TUNING_DATA_PORT = "TuningDataPort";
	const QString SINGLE_LM_CONTROL = "SingleLmControl";
	const QString DISABLE_MODULES_TYPE_CHECKING = "DisableModulesTypeChecking";

	// AppDataService properties
	//
	const QString APP_DATA_RECEIVING_NETMASK = "AppDataReceivingNetmask";
	const QString APP_DATA_RECEIVING_IP = "AppDataReceivingIP";
	const QString APP_DATA_RECEIVING_PORT = "AppDataReceivingPort";
	const QString RT_TRENDS_REQUEST_IP = "RtTrendsRequestIP";
	const QString RT_TRENDS_REQUEST_PORT = "RtTrendsRequestPort";
	const QString ARCH_SERVICE_ID = "ArchiveServiceID";
	const QString ARCH_SERVICE_IP = "ArchiveServiceIP";
	const QString ARCH_SERVICE_PORT = "ArchiveServicePort";
	const QString AUTO_ARCHIVE_INTERVAL = "AutoArchiveInterval";

	// DiagDataService properties
	//
	const QString DIAG_DATA_RECEIVING_NETMASK = "DiagDataReceivingNetmask";
	const QString DIAG_DATA_RECEIVING_IP = "DiagDataReceivingIP";
	const QString DIAG_DATA_RECEIVING_PORT = "DiagDataReceivingPort";

	// ArchivingService properties
	//
	const QString ARCHIVE_SHORT_TERM_PERIOD = "ShortTermArchivePeriod";
	const QString ARCHIVE_LONG_TERM_PERIOD = "LongTermArchivePeriod";
	const QString ARCHIVE_LOCATION = "ArchiveLocation";

	// Properties used in several Services
	//
	const QString CLIENT_REQUEST_IP = "ClientRequestIP";
	const QString CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";
	const QString CLIENT_REQUEST_PORT = "ClientRequestPort";

	const QString CFG_SERVICE_ID1 = "ConfigurationServiceID1";
	const QString CFG_SERVICE_IP1 = "ConfigurationServiceIP1";
	const QString CFG_SERVICE_PORT1 = "ConfigurationServicePort1";

	const QString CFG_SERVICE_ID2 = "ConfigurationServiceID2";
	const QString CFG_SERVICE_IP2 = "ConfigurationServiceIP2";
	const QString CFG_SERVICE_PORT2 = "ConfigurationServicePort2";
}

namespace XmlAttribute
{
	const QString COUNT("Count");
	const QString ID("ID");

	const QString DATA_ID("DataID");
	const QString HEX_DATA_ID("HexDataID");
}

namespace Latin1Char
{
	const QLatin1Char ZERO('0');
	const QLatin1Char SPACE(' ');
}


