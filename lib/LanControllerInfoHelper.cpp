#include "../Builder/IssueLogger.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/DataProtocols.h"
#include "../HardwareLib/DeviceObject.h"
#include "DeviceHelper.h"
#include "LanControllerInfoHelper.h"

// ---------------------------------------------------------------------------------
//
//	LanControllerInfoHelper class implementation
//
// ---------------------------------------------------------------------------------

const QString LanControllerInfoHelper::LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR("_ETHERNET0%1");

bool LanControllerInfoHelper::getInfo(	const Hardware::DeviceModule& lm,
										int lanControllerNo,
										E::LanControllerType lanControllerType,
										LanControllerInfo* lanControllerInfo,
										const Hardware::EquipmentSet& equipmentSet,
										Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lanControllerInfo, log);

	lanControllerInfo->controllerNo = lanControllerNo;
	lanControllerInfo->lanControllerType = lanControllerType;

	QString suffix = getLanControllerSuffix(lanControllerNo);

	Hardware::DeviceController* deviceController = DeviceHelper::getChildControllerBySuffix(&lm, suffix, log);

	if (deviceController == nullptr)
	{
		return false;
	}

	bool result = true;

	lanControllerInfo->equipmentID = deviceController->equipmentIdTemplate();

	lanControllerInfo->tuningProvided = false;
	lanControllerInfo->appDataProvided = false;
	lanControllerInfo->diagDataProvided = false;

	if (isProvideTuning(lanControllerType) == true)
	{
		// tuning adapter
		//
		lanControllerInfo->tuningProvided = true;

		result &= DeviceHelper::getBoolProperty(deviceController, EquipmentPropNames::TUNING_ENABLE,
												&lanControllerInfo->tuningEnable, log);
		QHostAddress tuningIP;

		result &= DeviceHelper::getIPv4Property(deviceController, EquipmentPropNames::TUNING_IP,
												&tuningIP, false, "", log);
		if (result == true)
		{
			lanControllerInfo->tuningIP = tuningIP.toString();
		}

		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::TUNING_PORT,
											   &lanControllerInfo->tuningPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::TUNING_SERVICE_ID,
											   &lanControllerInfo->tuningServiceID, log);

		lanControllerInfo->tuningServiceIP = Socket::IP_NULL;
		lanControllerInfo->tuningServicePort = 0;
		lanControllerInfo->tuningServiceNetmask = Socket::IP_NULL;

		if (lanControllerInfo->tuningEnable == true)
		{
			if (lanControllerInfo->tuningServiceID.isEmpty() == true)
			{
				// Property %1.%2 is empty.
				//
				log->errCFG3022(deviceController->equipmentIdTemplate(), EquipmentPropNames::TUNING_SERVICE_ID);
				result = false;
			}
			else
			{
				const Hardware::DeviceObject* tunService = equipmentSet.deviceObject(lanControllerInfo->tuningServiceID).get();

				if (tunService != nullptr)
				{
					result &= DeviceHelper::getStrProperty(tunService,
														   EquipmentPropNames::TUNING_DATA_IP,
														   &lanControllerInfo->tuningServiceIP, log);

					result &= DeviceHelper::getIntProperty(tunService,
														   EquipmentPropNames::TUNING_DATA_PORT,
														   &lanControllerInfo->tuningServicePort, log);

					result &= DeviceHelper::getStrProperty(tunService,
														   EquipmentPropNames::TUNING_DATA_NETMASK,
														   &lanControllerInfo->tuningServiceNetmask, log);
				}
			}
		}
	}

	if (isProvideAppData(lanControllerType) == true)
	{
		// application data adapter
		//
		lanControllerInfo->appDataProvided = true;

		result &= DeviceHelper::getBoolProperty(deviceController, EquipmentPropNames::APP_DATA_ENABLE,
												&lanControllerInfo->appDataEnable, log);
		QHostAddress appDataIP;

		result &= DeviceHelper::getIPv4Property(deviceController, EquipmentPropNames::APP_DATA_IP,
												&appDataIP, false, "", log);
		if (result == true)
		{
			lanControllerInfo->appDataIP = appDataIP.toString();
		}

		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::APP_DATA_PORT,
											   &lanControllerInfo->appDataPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::APP_DATA_SERVICE_ID,
											   &lanControllerInfo->appDataServiceID, log);
		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT,
												&lanControllerInfo->overrideAppDataWordCount, log);

		result &= DeviceHelper::getUIntProperty(&lm, EquipmentPropNames::LM_APP_LAN_DATA_UID,
												&lanControllerInfo->appDataUID, log);

		int appDataSizeW = 0;

		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::LM_APP_LAN_DATA_SIZE,
											   &appDataSizeW, log);

		lanControllerInfo->appDataSizeBytes = appDataSizeW * sizeof(quint16);

		lanControllerInfo->appDataFramesQuantity = lanControllerInfo->appDataSizeBytes / sizeof(Rup::Frame::data) +
													((lanControllerInfo->appDataSizeBytes % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);

		lanControllerInfo->appDataServiceIP = Socket::IP_NULL;
		lanControllerInfo->appDataServicePort = 0;
		lanControllerInfo->appDataServiceNetmask = Socket::IP_NULL;

		if (lanControllerInfo->appDataEnable == true)
		{
			const Hardware::DeviceObject* appDataService = equipmentSet.deviceObject(lanControllerInfo->appDataServiceID).get();

			if (appDataService != nullptr)
			{
				result &= DeviceHelper::getStrProperty(appDataService,
													   EquipmentPropNames::APP_DATA_RECEIVING_IP,
													   &lanControllerInfo->appDataServiceIP, log);

				result &= DeviceHelper::getIntProperty(appDataService,
													   EquipmentPropNames::APP_DATA_RECEIVING_PORT,
													   &lanControllerInfo->appDataServicePort, log);

				result &= DeviceHelper::getStrProperty(appDataService,
													   EquipmentPropNames::APP_DATA_RECEIVING_NETMASK,
													   &lanControllerInfo->appDataServiceNetmask, log);
			}
		}
	}

	if (isProvideDiagData(lanControllerType) == true)
	{
		// diagnostics data adapter
		//
		lanControllerInfo->diagDataProvided = true;

		result &= DeviceHelper::getBoolProperty(deviceController, EquipmentPropNames::DIAG_DATA_ENABLE,
												&lanControllerInfo->diagDataEnable, log);
		QHostAddress diagDataIP;

		result &= DeviceHelper::getIPv4Property(deviceController, EquipmentPropNames::DIAG_DATA_IP,
												&diagDataIP, false, "", log);
		if (result == true)
		{
			lanControllerInfo->diagDataIP = diagDataIP.toString();
		}

		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::DIAG_DATA_PORT,
											   &lanControllerInfo->diagDataPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::DIAG_DATA_SERVICE_ID,
											   &lanControllerInfo->diagDataServiceID, log);
		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT,
												&lanControllerInfo->overrideDiagDataWordCount, log);

		lanControllerInfo->diagDataUID = 0;
		lanControllerInfo->diagDataSizeBytes = 0;
		lanControllerInfo->diagDataFramesQuantity = 0;

/*		UNCOMMENT when LM will have PROP_LM_DIAG_DATA_UID and PROP_LM_DIAG_DATA_SIZE properties  !!!
 *
 * 		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_UID, &dataUID, log);

		diagDataUID = dataUID;

		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_SIZE, &diagDataSize, log);

		diagDataFramesQuantity = diagDataSize / sizeof(Rup::Frame::data) +
				((diagDataSize % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);
*/
		lanControllerInfo->diagDataServiceIP = Socket::IP_NULL;
		lanControllerInfo->diagDataServicePort = 0;
		lanControllerInfo->diagDataServiceNetmask = Socket::IP_NULL;

		if (lanControllerInfo->diagDataEnable == true)
		{
			const Hardware::DeviceObject* diagDataService = equipmentSet.deviceObject(lanControllerInfo->diagDataServiceID).get();

			if (diagDataService != nullptr)
			{
				result &= DeviceHelper::getStrProperty(diagDataService,
													   EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
													   &lanControllerInfo->diagDataServiceIP, log);

				result &= DeviceHelper::getIntProperty(diagDataService,
													   EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
													   &lanControllerInfo->diagDataServicePort, log);

				result &= DeviceHelper::getStrProperty(diagDataService,
													   EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK,
													   &lanControllerInfo->diagDataServiceNetmask, log);
			}
		}
	}

	return result;
}

bool LanControllerInfoHelper::isProvideTuning(E::LanControllerType lanControllerType)
{
	switch(lanControllerType)
	{
	case E::LanControllerType::Tuning:

		return true;

	case E::LanControllerType::AppData:
	case E::LanControllerType::DiagData:
	case E::LanControllerType::AppAndDiagData:

		return false;

	default:
		Q_ASSERT(false);
	}

	return false;
}

bool LanControllerInfoHelper::isProvideAppData(E::LanControllerType lanControllerType)
{
	switch(lanControllerType)
	{
	case E::LanControllerType::AppData:
	case E::LanControllerType::AppAndDiagData:

		return true;

	case E::LanControllerType::Tuning:
	case E::LanControllerType::DiagData:

		return false;

	default:
		Q_ASSERT(false);
	}

	return false;
}

bool LanControllerInfoHelper::isProvideDiagData(E::LanControllerType lanControllerType)
{
	switch(lanControllerType)
	{
	case E::LanControllerType::DiagData:
	case E::LanControllerType::AppAndDiagData:

		return true;

	case E::LanControllerType::Tuning:
	case E::LanControllerType::AppData:

		return false;

	default:
		Q_ASSERT(false);
	}

	return false;
}

QString LanControllerInfoHelper::getLanControllerSuffix(int controllerNo)
{
	return QString(LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR).arg(controllerNo);
}


