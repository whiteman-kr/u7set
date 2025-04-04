#include "../Builder/IssueLogger.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/SocketIO.h"
#include "../AppSignalLib/TuningDataStorage.h"

#include "DeviceHelper.h"
#include "LanControllerInfoHelper.h"

#include <HardwareLib/DataProtocols.h>
#include <HardwareLib/DeviceController.h>

// ---------------------------------------------------------------------------------
//
//	LanControllerInfoHelper class implementation
//
// ---------------------------------------------------------------------------------

const QString LanControllerInfoHelper::LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR("_ETHERNET0%1");

bool LanControllerInfoHelper::getInfo(const Hardware::DeviceModule& lm,
										E::LanControllerType lanControllerType,
										int lanControllerNo,
										const Builder::Context& context,
										bool ignoreTuningData,
										LanControllerInfo* lanControllerInfo,
										Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lanControllerInfo, log);

	Hardware::EquipmentSet& equipmentSet = *context.m_equipmentSet.get();

	lanControllerInfo->controllerNo = lanControllerNo;
	lanControllerInfo->lanControllerType = lanControllerType;

	QString suffix = getLanControllerSuffix(lanControllerNo);

	Hardware::DeviceController* deviceController = DeviceHelper::getChildControllerBySuffix(&lm, suffix, log);

	if (deviceController == nullptr)
	{
		// Can't find child controller with suffix %1 in object %2
		//
		log->errCFG3025(suffix, lm.equipmentIdTemplate());
		return false;
	}

	bool result = true;

	lanControllerInfo->equipmentID = deviceController->equipmentIdTemplate();

	if (LanControllerInfo::isProvideTuning(lanControllerType) == true)
	{
		// tuning adapter
		//
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

				if (tunService == nullptr)
				{
					// Property %1.%2 is linked to undefined software ID %3.
					//
					log->errCFG3021(deviceController->equipmentIdTemplate(),
									EquipmentPropNames::TUNING_SERVICE_ID,
									lanControllerInfo->tuningServiceID);

					return false;
				}

				if (tunService->isController() == false)
				{
					QStringList controllersIDs;

					if (DeviceHelper::isTwoChannelSoftware(tunService, &controllersIDs) == true)
					{
						// Property %1.%2 should refer to one of software controllers: %3
						//
						log->errCFG3047(deviceController->equipmentIdTemplate(),
										EquipmentPropNames::TUNING_SERVICE_ID,
										controllersIDs.join(Separator::COMMA_SPACE));

						return false;
					}
				}

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

				if (ignoreTuningData == true)
				{
					lanControllerInfo->rupTuningDataUID = 0;
					lanControllerInfo->fotipTuningDataUID = 0;
				}
				else
				{
					Tuning::TuningDataShared tuningData = context.m_tuningDataStorage->getTuningData(lm.equipmentIdTemplate());

					if (tuningData == nullptr)
					{
						// Tuning data is not found for module %1
						//
						log->errALC5197(lm.equipmentIdTemplate());
						result = false;
					}
					else
					{
						lanControllerInfo->rupTuningDataUID = tuningData->rupTuningDataUID();
						lanControllerInfo->fotipTuningDataUID = tuningData->fotipTuningDataUID();
					}
				}
			}
		}
	}

	if (LanControllerInfo::isProvideAppData(lanControllerType) == true)
	{
		// application data adapter
		//
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

		result &= DeviceHelper::getUIntProperty(&lm, EquipmentPropNames::APP_LAN_DATA_UID,
												&lanControllerInfo->rupAppDataUID, log);
		int appDataSizeW = 0;

		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::APP_LAN_DATA_SIZE,
											   &appDataSizeW, log);

		lanControllerInfo->appDataSizeBytes = appDataSizeW * sizeof(quint16);

		lanControllerInfo->appDataFramesQuantity =
				(lanControllerInfo->appDataSizeBytes + Rup::FRAME_DATA_SIZE - 1) / Rup::FRAME_DATA_SIZE;

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

	if (LanControllerInfo::isProvideDiagData(lanControllerType) == true)
	{
		// diagnostics data adapter
		//
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

		result &= DeviceHelper::getUIntProperty(&lm, EquipmentPropNames::DIAG_LAN_DATA_UID,
												&lanControllerInfo->rupDiagDataUID, log);
		int diagDataSizeW = 0;

		result &= DeviceHelper::getIntProperty(&lm, EquipmentPropNames::DIAG_LAN_DATA_SIZE,
											   &diagDataSizeW, log);

		lanControllerInfo->diagDataSizeBytes = diagDataSizeW * sizeof(quint16);

		lanControllerInfo->diagDataFramesQuantity =
				(lanControllerInfo->diagDataSizeBytes + Rup::FRAME_DATA_SIZE - 1) / Rup::FRAME_DATA_SIZE;

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

bool LanControllerInfoHelper::getInfo(const Hardware::DeviceModule& lm,
									E::LanControllerType lanControllerType,
									const Builder::Context& context,
									bool ignoreTuningData,
									LanControllersInfo* lanControllersInfo,
									Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lanControllersInfo, log);

	std::shared_ptr<LmDescription> lmDescription = context.m_lmDescriptions->get(&lm);

	if (lmDescription == nullptr)
	{
		LOG_INTERNAL_ERROR_MSG(log, QString("LmDescription is not found for module %1").arg(lm.equipmentIdTemplate()));
		return false;
	}

	lanControllersInfo->clear();

	const LmDescription::Lan& lan = lmDescription->lan();

	lanControllersInfo->setRupVersion(lan.m_rupVersion);
	lanControllersInfo->setFotipVersion(lan.m_fotipVersion);

	bool result = true;

	for(const LmDescription::LanController& lanController : lan.m_lanControllers)
	{
		if ((TO_INT(lanController.m_type) & (TO_INT(lanControllerType))) == 0)
		{
			// this lanController is not provide function selected by lanControllerType
			continue;
		}

		LanControllerInfo lanInfo;

		result &= getInfo(lm, lanControllerType, lanController.m_place,
						  context, ignoreTuningData, &lanInfo, log);

		if (result == true)
		{
			lanControllersInfo->append(lanInfo);
		}
	}

	return result;
}

QString LanControllerInfoHelper::getLanControllerSuffix(int controllerNo)
{
	return QString(LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR).arg(controllerNo);
}
