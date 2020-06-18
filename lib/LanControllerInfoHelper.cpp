#include "LanControllerInfoHelper.h"

// ---------------------------------------------------------------------------------
//
//	LanControllerInfoHelper class implementation
//
// ---------------------------------------------------------------------------------

#include "../Builder/IssueLogger.h"
#include "DeviceObject.h"
#include "WUtils.h"
#include "DeviceHelper.h"
#include "DataProtocols.h"

const QString LanControllerInfoHelper::LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR("_ETHERNET0%1");

const QString LanControllerInfoHelper::IP_NULL("0.0.0.0");

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
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::TUNING_IP,
											   &lanControllerInfo->tuningIP, log);
		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::TUNING_PORT,
											   &lanControllerInfo->tuningPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::TUNING_SERVICE_ID,
											   &lanControllerInfo->tuningServiceID, log);


		lanControllerInfo->tuningServiceIP = IP_NULL;
		lanControllerInfo->tuningServicePort = 0;

		if (lanControllerInfo->tuningEnable == true &&
			lanControllerInfo->tuningServiceID.isEmpty() == false)
		{
			const Hardware::DeviceObject* tunService = equipmentSet.deviceObject(lanControllerInfo->tuningServiceID);

			if (tunService == nullptr)
			{
				// Property %1.%2 is linked to undefined software ID %3.
				//
				log->errCFG3021(lanControllerInfo->equipmentID, EquipmentPropNames::TUNING_SERVICE_ID, lanControllerInfo->tuningServiceID);
				result = false;
			}
			else
			{
				result &= DeviceHelper::getStrProperty(tunService,
													   EquipmentPropNames::TUNING_DATA_IP,
													   &lanControllerInfo->tuningServiceID, log);

				result &= DeviceHelper::getIntProperty(tunService,
													   EquipmentPropNames::TUNING_DATA_PORT,
													   &lanControllerInfo->tuningServicePort, log);

				result &= DeviceHelper::getStrProperty(tunService,
													   EquipmentPropNames::TUNING_DATA_NETMASK,
													   &lanControllerInfo->tuningServiceNetmask, log);
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
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::APP_DATA_IP,
											   &lanControllerInfo->appDataIP, log);
		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::APP_DATA_PORT,
											   &lanControllerInfo->appDataPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::APP_DATA_SERVICE_ID,
											   &lanControllerInfo->appDataServiceID, log);
		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT,
												&lanControllerInfo->overrideAppDataWordCount, log);

		result &= DeviceHelper::getUIntProperty(&lm, EquipmentPropNames::LM_APP_LAN_DATA_UID,
												&lanControllerInfo->appDataUID, log);

		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::LM_APP_LAN_DATA_SIZE,
											   &lanControllerInfo->appDataSize, log);

		lanControllerInfo->appDataSize *= sizeof(quint16);		// size in words convert to size in bytes

		lanControllerInfo->appDataFramesQuantity = lanControllerInfo->appDataSize / sizeof(Rup::Frame::data) +
													((lanControllerInfo->appDataSize % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);

	}

	if (isProvideDiagData(lanControllerType) == true)
	{
		// diagnostics data adapter
		//
		lanControllerInfo->diagDataProvided = true;

		result &= DeviceHelper::getBoolProperty(deviceController, EquipmentPropNames::DIAG_DATA_ENABLE,
												&lanControllerInfo->diagDataEnable, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::DIAG_DATA_IP,
											   &lanControllerInfo->diagDataIP, log);
		result &= DeviceHelper::getIntProperty(deviceController, EquipmentPropNames::DIAG_DATA_PORT,
											   &lanControllerInfo->diagDataPort, log);
		result &= DeviceHelper::getStrProperty(deviceController, EquipmentPropNames::DIAG_DATA_SERVICE_ID,
											   &lanControllerInfo->diagDataServiceID, log);
		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT,
												&lanControllerInfo->overrideDiagDataWordCount, log);

		lanControllerInfo->diagDataUID = 0;
		lanControllerInfo->diagDataSize = 0;
		lanControllerInfo->diagDataFramesQuantity = 0;

/*		UNCOMMENT when LM will have PROP_LM_DIAG_DATA_UID and PROP_LM_DIAG_DATA_SIZE properties  !!!
 *
 * 		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_UID, &dataUID, log);

		diagDataUID = dataUID;

		result &= DeviceHelper::getIntProperty(lm, PROP_LM_DIAG_DATA_SIZE, &diagDataSize, log);

		diagDataFramesQuantity = diagDataSize / sizeof(Rup::Frame::data) +
				((diagDataSize % sizeof(Rup::Frame::data)) == 0 ? 0 : 1);
*/
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


