#include "SoftwareSettingsGetter.h"

#include <HardwareLib/LanControllerInfo.h>
#include <HardwareLib/DeviceController.h>
#include <HardwareLib/Workstation.h>

#include "../lib/DataSource.h"
#include "../TuningService/TuningSource.h"

#include "LanControllerInfoHelper.h"

// -------------------------------------------------------------------------------------
//
// ServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

SoftwareSettingsGetter::~SoftwareSettingsGetter()
{
}

bool SoftwareSettingsGetter::getSoftwareConnection(const Hardware::EquipmentSet* equipment,
												   const Hardware::Software* thisSoftware,
												   const QString& propConnectedSoftwareID,
												   const QString& propConnectedSoftwareIP,
												   const QString& propConnectedSoftwarePort,
												   QString* connectedSoftwareID,
												   HostAddressPort* connectedSoftwareIP,
												   bool emptyAllowed,
												   const QString& defaultIP,
												   int defaultPort,
												   E::SoftwareType requiredSoftwareType,
												   Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);
	TEST_PTR_LOG_RETURN_FALSE(thisSoftware, log);
	TEST_PTR_LOG_RETURN_FALSE(connectedSoftwareID, log);
	TEST_PTR_LOG_RETURN_FALSE(connectedSoftwareIP, log);

	if (emptyAllowed == true)
	{
		QHostAddress addr;

		if (addr.setAddress(defaultIP) == false)
		{
			LOG_INTERNAL_ERROR(log);
			return false;
		}

		if (defaultPort < Socket::PORT_LOWEST || defaultPort > Socket::PORT_HIGHEST)
		{
			LOG_INTERNAL_ERROR(log);
			return false;
		}
	}

	bool result = true;

	result = DeviceHelper::getStrProperty(thisSoftware, propConnectedSoftwareID, connectedSoftwareID, log);

	if (result == false)
	{
		return false;
	}

	*connectedSoftwareID = connectedSoftwareID->trimmed();

	if (connectedSoftwareID->isEmpty() == true &&
			emptyAllowed == false)
	{
		//  Property '%1.%2' is empty.
		//
		log->errCFG3022(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);

		return false;
	}

	return getSoftwareConnectionBySoftwareID(equipment,
											 thisSoftware,
											 *connectedSoftwareID,
											 propConnectedSoftwareID,
											 propConnectedSoftwareIP,
											 propConnectedSoftwarePort,
											 connectedSoftwareIP,
											 emptyAllowed,
											 defaultIP,
											 defaultPort,
											 requiredSoftwareType,
											 log);
}


bool SoftwareSettingsGetter::getSoftwareConnectionBySoftwareID(const Hardware::EquipmentSet* equipment,
															   const Hardware::Software* thisSoftware,
															   const QString& connectedSoftwareID,
															   const QString& propConnectedSoftwareID,
															   const QString& propConnectedSoftwareIP,
															   const QString& propConnectedSoftwarePort,
															   HostAddressPort* connectedSoftwareIP,
															   bool emptyAllowed,
															   const QString& defaultIP,
															   int defaultPort,
															   E::SoftwareType requiredSoftwareType,
															   Builder::IssueLogger* log)
{
	bool result = true;

	if (connectedSoftwareID.isEmpty() == true)
	{
		if (emptyAllowed == true)
		{
			//  Property '%1.%2' is empty.
			//
			//	warning removed RPCT-3763
			//
			//	log->wrnCFG3016(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);

			connectedSoftwareIP->setAddressPort(defaultIP, defaultPort);

			return true;
		}

		//  Property '%1.%2' is empty.
		//
		log->errCFG3022(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);

		return false;
	}

	const Hardware::Software* connectedSoftware = nullptr;

	const std::shared_ptr<Hardware::DeviceObject> sharedConnectedObject = equipment->deviceObject(connectedSoftwareID);

	if (sharedConnectedObject == nullptr)
	{
		// Property '%1.%2' is linked to undefined software ID '%3'.
		//
		log->errCFG3021(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID, connectedSoftwareID);
		return false;
	}

	const Hardware::DeviceObject* connectedObject = sharedConnectedObject.get();

	if (connectedObject->isSoftware() == true)
	{
		connectedSoftware = connectedObject->toSoftware().get();
	}
	else
	{
		if (connectedObject->isController() == true)
		{
			connectedSoftware = connectedObject->getParentSoftware();
		}
		else
		{
			// Property %1.%2 should refer to Software or Software child controller object.
			//
			log->errCFG3048(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);
			return false;
		}
	}

	if (connectedSoftware == nullptr)
	{
		// Property '%1.%2' is linked to undefined software ID '%3'.
		//
		log->errCFG3021(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID, connectedSoftwareID);
		return false;
	}

	if (requiredSoftwareType != E::SoftwareType::Unknown)
	{
		if (connectedSoftware->softwareType() != requiredSoftwareType)
		{
			// Property %1.%2 is linked to not compatible software ID %3.
			//
			log->errCFG3017(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID, connectedSoftware->equipmentIdTemplate());
			return false;
		}
	}

	result = DeviceHelper::getIpPortProperty(	connectedObject,
												propConnectedSoftwareIP,
												propConnectedSoftwarePort,
												connectedSoftwareIP,
												emptyAllowed, defaultIP, defaultPort, log);
	return result;
}

bool SoftwareSettingsGetter::getCfgServiceConnection(	const Hardware::EquipmentSet* equipment,
												const Hardware::Software* software,
												QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
												QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
												Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);
	TEST_PTR_LOG_RETURN_FALSE(cfgServiceID1, log);
	TEST_PTR_LOG_RETURN_FALSE(cfgServiceAddrPort1, log);
	TEST_PTR_LOG_RETURN_FALSE(cfgServiceID2, log);
	TEST_PTR_LOG_RETURN_FALSE(cfgServiceAddrPort2, log);

	bool result = true;

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::CFG_SERVICE_ID1,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									cfgServiceID1,
									cfgServiceAddrPort1,
									true, Socket::IP_NULL,
									PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::ConfigurationService, log);

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::CFG_SERVICE_ID2,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									cfgServiceID2,
									cfgServiceAddrPort2,
									true, Socket::IP_NULL,
									PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::ConfigurationService, log);
	if (result == false)
	{
		return false;
	}

	if (cfgServiceID1->isEmpty() == true && cfgServiceID2->isEmpty() == true)
	{
		// Software %1 is not linked to ConfigurationService.
		//
		log->errCFG3029(software->equipmentIdTemplate());
		return false;
	}

	return result;
}

bool SoftwareSettingsGetter::getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
													const Hardware::Software* software,
													SoftwareEndpoint::ConfigService* cfgService1,
													SoftwareEndpoint::ConfigService* cfgService2,
													Builder::IssueLogger* log)
{
	TEST_PTR_LOG_RETURN_FALSE(cfgService1, log);
	TEST_PTR_LOG_RETURN_FALSE(cfgService2, log);

	return getCfgServiceConnection(equipment,
								   software,
								   &cfgService1->equipmentId, &cfgService1->address,
								   &cfgService2->equipmentId, &cfgService2->address,
								   log);
}

bool SoftwareSettingsGetter::getLmPropertiesFromDevice(	const Hardware::DeviceModule* lm,
														E::LanControllerType lanControllerType,
														const Builder::Context* context,
														DataSource* ds)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(lm, log);
	TEST_PTR_LOG_RETURN_FALSE(ds, log);

	ds->setModuleEquipmentID(lm->equipmentIdTemplate());
	ds->setModulePresetName(lm->presetName());
	ds->setModuleType(lm->moduleType());
	ds->setModuleCaption(lm->caption());

	bool result = true;

	int lmNumber = 0;
	QString subsystemChannel;
	QString subsystemID;

	result &= DeviceHelper::getIntProperty(lm, EquipmentPropNames::LM_NUMBER, &lmNumber, log);
	result &= DeviceHelper::getStrProperty(lm, EquipmentPropNames::SUBSYSTEM_CHANNEL, &subsystemChannel, log);
	result &= DeviceHelper::getStrProperty(lm, EquipmentPropNames::SUBSYSTEM_ID, &subsystemID, log);

	ds->setLmNumber(lmNumber);
	ds->setSubsystemChannel(subsystemChannel);
	ds->setSubsystemID(subsystemID);

	int subsystemKey = context->m_subsystems->subsystemKey(subsystemID);

	if (subsystemKey == -1)
	{
		// Subsystem '%1' is not found in subsystem set (Logic Module '%2')
		//
		log->errCFG3001(subsystemID, lm->equipmentIdTemplate());
		return false;
	}

	ds->setSubsystemKey(subsystemKey);

	auto pos = context->m_lmsUniqueIDs.find(lm->equipmentIdTemplate());

	if (pos != context->m_lmsUniqueIDs.end())
	{
		ds->setModuleUniqueID(pos->second);
	}
	else
	{
		ds->setModuleUniqueID(0);
	}

	std::shared_ptr<LmDescription> ld = context->m_lmDescriptions->get(lm);

	if (ld == nullptr)
	{
		LOG_INTERNAL_ERROR_MSG(log, QString("LmDescription is not found for module %1").
											arg(lm->equipmentIdTemplate()));
		return false;
	}

	ds->setModuleWorkcycle_mcs(ld->logicUnit().m_cycleDuration);

	result &= LanControllerInfoHelper::getInfo(*lm, lanControllerType,
											   *context, false,
											   &ds->lanControllersInfo(), log);
	return result;
}

bool SoftwareSettingsGetter::readSoftwareSettings(const Builder::Context* context,
												  const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);
	TEST_PTR_RETURN_FALSE(software);

	const Hardware::Workstation* ws = software->getParentWorkstation();

	if (ws == nullptr)
	{
		Q_ASSERT(false);
		LOG_INTERNAL_ERROR_MSG(context->m_log, QString("Software %1 hasn't parent Workstation").
							   arg(software->equipmentIdTemplate()));
		return false;
	}

	hostname = ws->hostname();

	return readSettings(context, software);
}

bool SoftwareSettingsGetter::readFromDeviceByEquipmentID(const Builder::Context* context,
														 const QString& softwareID,
														 E::SoftwareType requiredSoftwareType)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);

	const std::shared_ptr<Hardware::EquipmentSet> equipmentSet = context->m_equipmentSet;

	TEST_PTR_LOG_RETURN_FALSE(equipmentSet, log);

	const std::shared_ptr<Hardware::DeviceObject> deviceObject = equipmentSet->deviceObject(softwareID);

	if (deviceObject == nullptr)
	{
		// Device object %1 not found.
		//
		log->errEQP6010(softwareID);
		return false;
	}

	if (deviceObject->isSoftware() == false)
	{
		LOG_INTERNAL_ERROR_MSG(log, QString("Device object %1 is not Hardware::Software type").
							   arg(softwareID));
		return false;
	}

	const std::shared_ptr<Hardware::Software> software = deviceObject->toSoftware();

	if (software == nullptr)
	{
		LOG_INTERNAL_ERROR(log);
		return false;
	}

	if(requiredSoftwareType != E::SoftwareType::Unknown)
	{
		if (software->softwareType() != requiredSoftwareType)
		{
			LOG_INTERNAL_ERROR_MSG(log, QString("Unappropriate software type of %1, required %2").
								   arg(softwareID).arg(E::valueToString<E::SoftwareType>(requiredSoftwareType)));
			return false;
		}
	}

	return readSettings(context, software.get());
}


// -------------------------------------------------------------------------------------
//
// CfgServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServiceSettingsGetter::readSettings(	const Builder::Context* context,
												const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	bool result = true;

	result &= DeviceHelper::getIpPortProperty(software, EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP, false, "", 0, log);
	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask, false, "", log);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::CHECK_HOSTNAME, &checkHostname, log);

	result &= DeviceHelper::getEnumValueProperty<E::SecurityLevel>(software, EquipmentPropNames::SECURITY_LEVEL, &securityLevel, log);

	RETURN_IF_FALSE(result);

	result &= buildClientsList(context, software);

	return result;
}

bool CfgServiceSettingsGetter::buildClientsList(const Builder::Context* context, const Hardware::Software* cfgService)
{
	const QString PROP_CFG_SERVICE_ID1(EquipmentPropNames::CFG_SERVICE_ID1);
	const QString PROP_CFG_SERVICE_ID2(EquipmentPropNames::CFG_SERVICE_ID2);
	const QString PROP_CFG_SERVICE_IDS(EquipmentPropNames::CFG_SERVICE_IDS);

	Builder::IssueLogger* log = context->m_log;

	bool result = true;

	clients.clear();

	std::set<const Hardware::Workstation*> reportedWs;

	for(auto p : context->m_software)
	{
		Hardware::Software* software = p.second;

		if (software == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		if (software->equipmentIdTemplate() == cfgService->equipmentIdTemplate())
		{
			continue;			// exclude yourself
		}

		QString ID1;
		QString ID2;

		if (DeviceHelper::isPropertyExists(software, PROP_CFG_SERVICE_IDS) == true)
		{
			QStringList ids;
			result &= DeviceHelper::getStrListProperty(software, PROP_CFG_SERVICE_IDS, &ids, log);

			if (ids.size() == 1)
			{
				ID1 = ids[0];
			}
			else
			{
				if (ids.size() >= 2)
				{
					ID1 = ids[0];
					ID2 = ids[1];
				}
			}
		}

		//

		if (DeviceHelper::isPropertyExists(software, PROP_CFG_SERVICE_ID1) == true)
		{
			result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID1, &ID1, log);
		}

		if (DeviceHelper::isPropertyExists(software, PROP_CFG_SERVICE_ID2) == true)
		{
			result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID2, &ID2, log);
		}

		//

		if (ID1 == cfgService->equipmentIdTemplate() || ID2 == cfgService->equipmentIdTemplate())
		{
			QString hostname = software->hostname();

			if (checkHostname == true && hostname.isEmpty() == true)
			{
				const Hardware::Workstation* ws = software->getParentWorkstation();

				if (ws != nullptr)
				{
					if (reportedWs.find(ws) == reportedWs.end())
					{
						// %1.CheckHostname is set True but hostname of workstation %2 isn't set.
						//
						log->errCFG3049(cfgService->equipmentIdTemplate(), ws->equipmentIdTemplate());
						reportedWs.insert(ws);

						result = false;
					}
				}
				else
				{
					Q_ASSERT(false);
					LOG_NULLPTR_ERROR(log);
				}
			}

			clients.append({.equipmentID = software->equipmentIdTemplate(),
							.softwareType = software->softwareType(),
							.hostname = hostname});
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// AppDataServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool AppDataServiceSettingsGetter::readSettings(const Builder::Context* context,
												const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::APP_DATA_RECEIVING_IP,
											  EquipmentPropNames::APP_DATA_RECEIVING_PORT,
											  &appDataReceivingIP,
											  false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::APP_DATA_RECEIVING_NETMASK,
											&appDataReceivingNetmask, false, "", log);

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT,
											  &clientRequestIP,
											  false, "", 0, log);

	int rtTrendsRequestPort = 0;

	result &= DeviceHelper::getPortProperty(software, EquipmentPropNames::RT_TRENDS_REQUEST_PORT,
											&rtTrendsRequestPort, true, PORT_APP_DATA_SERVICE_RT_TRENDS_REQUEST, log);

	rtTrendsRequestIP.setAddressPort(clientRequestIP.addressStr(), rtTrendsRequestPort);

	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK,
											&clientRequestNetmask, false, "", log);

	result &= DeviceHelper::getEnumValueProperty<E::SecurityLevel>(software, EquipmentPropNames::SECURITY_LEVEL, &securityLevel, log);

	result &= getSoftwareConnection(equipment, software,
									EquipmentPropNames::ARCH_SERVICE_ID,
									EquipmentPropNames::APP_DATA_RECEIVING_IP,
									EquipmentPropNames::APP_DATA_RECEIVING_PORT,
									&archServiceID,	&archServiceIP,
									true, Socket::IP_NULL,
									PORT_ARCHIVING_SERVICE_APP_DATA,
									E::SoftwareType::ArchiveService, log);

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);

	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::AUTO_ARCHIVE_INTERVAL,
										   &autoArchiveInterval, log);
	return result;
}

// -------------------------------------------------------------------------------------
//
// DiagDataServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool DiagDataServiceSettingsGetter::readSettings(const Builder::Context* context,
												 const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
											  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
											  &diagDataReceivingIP,
											  false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK,
											&diagDataReceivingNetmask, false, "", log);

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT,
											  &clientRequestIP,
											  false, "", 0, log);

	int rtTrendsRequestPort = 0;

	result &= DeviceHelper::getPortProperty(software, EquipmentPropNames::RT_TRENDS_REQUEST_PORT,
											&rtTrendsRequestPort, true, PORT_DIAG_DATA_SERVICE_RT_TRENDS_REQUEST, log);

	rtTrendsRequestIP.setAddressPort(clientRequestIP.addressStr(), rtTrendsRequestPort);


	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK,
											&clientRequestNetmask, false, "", log);

	result &= DeviceHelper::getEnumValueProperty<E::SecurityLevel>(software, EquipmentPropNames::SECURITY_LEVEL, &securityLevel, log);

	result &= getSoftwareConnection(equipment, software,
									EquipmentPropNames::ARCH_SERVICE_ID,
									EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
									&archServiceID,	&archServiceIP,
									true, Socket::IP_NULL,
									PORT_ARCHIVING_SERVICE_DIAG_DATA,
									E::SoftwareType::ArchiveService, log);

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);
	return result;
}

// -------------------------------------------------------------------------------------
//
// TuningServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool TuningServiceSettingsGetter::readSettings(const Builder::Context* context,
											   const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;

	equipmentID = software->equipmentIdTemplate();

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT,
											  &clientRequestIP, false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software,
											EquipmentPropNames::CLIENT_REQUEST_NETMASK,
											&clientRequestNetmask, false, "", log);

	result &= DeviceHelper::getEnumValueProperty<E::SecurityLevel>(software, EquipmentPropNames::SECURITY_LEVEL, &securityLevel, log);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl, log);
	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking, log);

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);

	std::vector<const Hardware::DeviceController*> controllers;

	bool hasControllers = true;

	for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
	{
		const Hardware::DeviceController* controller =
				DeviceHelper::getChildControllerBySuffix(software,
														 EquipmentPropNames::CONTROLLER_SUFFIX_CH_TEMPLATE.arg(channel + 1));
		if (controller == nullptr)
		{
			hasControllers = false;
		}

		controllers.push_back(controller);
	}

	if (hasControllers == true)
	{
		isTwoChannelTuningService = true;

		channelCount = TuningServiceSettings::CHANNELS_COUNT;

		for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
		{
			const Hardware::DeviceController* controller = controllers[channel];
			ChannelSettings& ch = channelSettings[channel];

			ch.serviceControllerEquipmentID = controller->equipmentIdTemplate();

			result &= DeviceHelper::getBoolProperty(controller,
													EquipmentPropNames::ENABLE,
													&ch.enable, log);

			result &= DeviceHelper::getIpPortProperty(controller,
													  EquipmentPropNames::TUNING_DATA_IP,
													  EquipmentPropNames::TUNING_DATA_PORT,
													  &ch.tuningDataIP, false, "", 0, log);

			result &= DeviceHelper::getIPv4Property(controller,
													EquipmentPropNames::TUNING_DATA_NETMASK,
													&ch.tuningDataNetmask, false, "", log);

			result &= DeviceHelper::getIpPortProperty(controller,
													  EquipmentPropNames::TUNING_SIM_IP,
													  EquipmentPropNames::TUNING_SIM_PORT,
													  &ch.tuningSimIP, true, Socket::IP_LOCALHOST,
													  PORT_LM_TUNING, log);
		}
	}
	else
	{
		// Reading of single-channel TuningService preset
		//
		isTwoChannelTuningService = false;

		channelCount = 1;

		ChannelSettings& ch1 = channelSettings[0];

		ch1.enable = true;

		ch1.serviceControllerEquipmentID = equipmentID;

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::TUNING_DATA_IP,
												  EquipmentPropNames::TUNING_DATA_PORT,
												  &ch1.tuningDataIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::TUNING_DATA_NETMASK,
												&ch1.tuningDataNetmask, false, "", log);

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::TUNING_SIM_IP,
												  EquipmentPropNames::TUNING_SIM_PORT,
												  &ch1.tuningSimIP, true, Socket::IP_LOCALHOST,
												  PORT_LM_TUNING, log);

		ChannelSettings& ch2 = channelSettings[1];

		ch2.enable = false;

		ch2.serviceControllerEquipmentID.clear();
		ch2.tuningDataIP = HostAddressPort();
		ch2.tuningDataNetmask = QHostAddress();
		ch2.tuningSimIP = HostAddressPort();
	}

	//
	// Next checkings is valid for CHANNELS_COUNT == 2 only!
	//
	Q_ASSERT(CHANNELS_COUNT == 2);

	ChannelSettings& ch1 = channelSettings[CHANNEL_1];
	ChannelSettings& ch2 = channelSettings[CHANNEL_2];

	if (ch1.enable && ch2.enable)
	{
		if (ch1.tuningDataIP.addressPortStr() == ch2.tuningDataIP.addressPortStr())
		{
			// Value of properties pair %1:%2 of objects %3 and %4 are equal
			//
			log->errCFG3046(EquipmentPropNames::TUNING_DATA_IP,
							EquipmentPropNames::TUNING_DATA_PORT,
							ch1.serviceControllerEquipmentID,
							ch2.serviceControllerEquipmentID);
			result = false;
		}

		if (ch1.tuningSimIP.addressPortStr() == ch2.tuningSimIP.addressPortStr())
		{
			// Value of properties pair %1:%2 of objects %3 and %4 are equal
			//
			log->errCFG3046(EquipmentPropNames::TUNING_SIM_IP,
							EquipmentPropNames::TUNING_SIM_PORT,
							ch1.serviceControllerEquipmentID,
							ch2.serviceControllerEquipmentID);
			result = false;
		}

		RETURN_IF_FALSE(result);
	}

	//

	for(int channel = CHANNEL_1; channel < CHANNELS_COUNT; channel++)
	{
		channelSettings[channel].sources.clear();

		if (channelSettings[channel].enable == false)
		{
			continue;
		}

		result &= fillTuningSourcesInfo(context, channel);
	}

	result &= fillMatsUsers(context);
	result &= fillTuningClientsInfo(context, software, singleLmControl);

	if (context->m_projectProperties.safetyProject() == true && singleLmControl == false)
	{
		// TuningService (%1) cannot be used for multi LM control in Safety Project. Turn On option %1.SingleLmControl or override behaviour in menu Project->Project Properties...->Safety Project.
		//
		log->errEQP6201(software->equipmentIdTemplate());
		return false;
	}

	return result;
}

bool TuningServiceSettingsGetter::fillTuningSourcesInfo(const Builder::Context* context,
														int channel)
{
	Q_ASSERT(channel >=0 && channel < CHANNELS_COUNT);

	Builder::IssueLogger* log = context->m_log;

	bool result = true;

	ChannelSettings& ch = channelSettings[channel];

	QString srvControllerEquipmentID = ch.serviceControllerEquipmentID;

	quint32 receivingNetmask = ch.tuningDataNetmask.toIPv4Address();

	quint32 receivingSubnet = ch.tuningDataIP.address32() & receivingNetmask;

	HostAddressPort tuningDataIP = ch.tuningDataIP;

	std::vector<TuningSource>& srcs = ch.sources;

	for(Hardware::DeviceModule* lm : context->m_lmModules)
	{
		if (lm == nullptr)
		{
			LOG_NULLPTR_ERROR(log);
			result = false;
			continue;
		}

		Tuning::TuningSource ts;

		bool res = getLmPropertiesFromDevice(lm, E::LanControllerType::Tuning,
											 context, &ts);
		if (res == false)
		{
			result = false;
			continue;
		}

		for(const LanControllerInfo& lan : ts.lanControllersInfo()())
		{
			if (lan.tuningEnable == false || lan.tuningServiceID != srvControllerEquipmentID)
			{
				continue;
			}

			if ((QHostAddress(lan.tuningIP).toIPv4Address() & receivingNetmask) != receivingSubnet)
			{
				// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
				//
				log->errCFG3043(lan.tuningIP,
								lan.equipmentID,
								tuningDataIP.addressStr(),
								srvControllerEquipmentID);
				result = false;
				continue;
			}

			TuningServiceSettings::TuningSource tunSrc;

			tunSrc.lmEquipmentID = ts.moduleEquipmentID();
			tunSrc.portEquipmentID = lan.equipmentID;
			tunSrc.tuningDataIP = HostAddressPort(lan.tuningIP, lan.tuningPort);

			srcs.push_back(tunSrc);
		}
	}

	return result;
}

bool TuningServiceSettingsGetter::fillTuningClientsInfo(const Builder::Context* context,
														const Hardware::Software* software,
														bool singleLmControlEnabled)
{
	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);

	clients.clear();

	bool result = true;

	QString thisTuningServiceID = equipmentID;

	static const std::set<E::SoftwareType> knownClientSoftwareTypes =
	{
		E::SoftwareType::TuningClient,
		E::SoftwareType::Metrology,
		E::SoftwareType::Monitor,
		E::SoftwareType::TestClient,
		E::SoftwareType::TestSuite
	};

	for(const auto& p : context->m_software)
	{
		const Hardware::Software* tuningClient = p.second;

		TEST_PTR_CONTINUE(tuningClient);

		if (knownClientSoftwareTypes.contains(tuningClient->softwareType()) == false)
		{
			continue;
		}

		QStringList tuningServicesIDs;

		result &= DeviceHelper::getStrListProperty(tuningClient, EquipmentPropNames::TUNING_SERVICE_ID,
												   &tuningServicesIDs, log);

		if (result == false)
		{
			continue;
		}

		if (tuningServicesIDs.contains(thisTuningServiceID) == false)
		{
			continue;
		}

		bool tuningEnable = true;			// by default tuning is enabled for known clients without property "TuningEnable"

		if (DeviceHelper::isPropertyExists(tuningClient, EquipmentPropNames::TUNING_ENABLE) == true)
		{
			result &= DeviceHelper::getBoolProperty(tuningClient, EquipmentPropNames::TUNING_ENABLE, &tuningEnable, log);

			if (result == false)
			{
				continue;
			}

			if (tuningEnable == false)
			{
				continue;
			}

			if (tuningClient->softwareType() == E::SoftwareType::Monitor && singleLmControlEnabled == true)
			{
				// Monitor %1 cannot be connected to TuningService %2 with enabled SingleLmControl mode.
				//
				log->errALC5150(tuningClient->equipmentIdTemplate(), thisTuningServiceID);
				result = false;
				continue;
			}
		}

		// TuningClient is linked to this TuningService

		TuningClient tunClient;

		tunClient.equipmentID = tuningClient->equipmentIdTemplate();
		tunClient.softwareType = tuningClient->softwareType();

		//

		if (DeviceHelper::isPropertyExists(tuningClient, EquipmentPropNames::TUNING_LOGIN) == true)
		{
			result &= DeviceHelper::getBoolProperty(tuningClient, EquipmentPropNames::TUNING_LOGIN, &tunClient.tuningLogin, log);
		}
		else
		{
			if (DeviceHelper::isPropertyExists(tuningClient, EquipmentPropNames::TESTING_LOGIN) == true)
			{
				result &= DeviceHelper::getBoolProperty(tuningClient, EquipmentPropNames::TESTING_LOGIN, &tunClient.tuningLogin, log);
			}
			else
			{
				tunClient.tuningLogin = false;
			}
		}

		//

		QString matsUserAccountsPropName;

		switch(tunClient.softwareType)
		{
		case E::SoftwareType::Monitor:
		case E::SoftwareType::TuningClient:
			matsUserAccountsPropName = EquipmentPropNames::TUNING_USER_ACCOUNTS;
			break;

		case E::SoftwareType::TestSuite:
			matsUserAccountsPropName = EquipmentPropNames::TESTING_USER_ACCOUNTS;
			break;

		case E::SoftwareType::Metrology:
		case E::SoftwareType::TestClient:
			break;

		default:
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR_MSG(log, QString("Unknown software type %1 (%2)").
											arg(E::valueToString(tuningClient->softwareType())).
											arg(TO_INT(tuningClient->softwareType())));
			result = false;
		}

		if (matsUserAccountsPropName.isEmpty() == false)
		{
			if (DeviceHelper::isPropertyExists(tuningClient, matsUserAccountsPropName) == true)
			{
				result &= DeviceHelper::getStrListPropertyAsString(tuningClient, matsUserAccountsPropName, &tunClient.matsUsers, log);
			}
			else
			{
				// Property %1.%2 is not found.
				//
				log->errCFG3020(tuningClient->equipmentIdTemplate(), matsUserAccountsPropName);
				result = false;
			}
		}

		//

		QStringList sourcesIDs;

		result &= DeviceHelper::getStrListProperty(tuningClient, EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID,
												   &sourcesIDs, log);

		if (sourcesIDs.isEmpty() == true)
		{
			if (context->m_projectProperties.safetyProject() == true)
			{
				// %1.TuningSourceEquipmentID property can't be empty in Safety Project. Specify tuning sources which are processed by this client.
				//
				log->errEQP6204(tuningClient->equipmentIdTemplate());
				result = false;
			}
			else
			{
				for(int ch = CHANNEL_1; ch < CHANNELS_COUNT; ch++)
				{
					tunClient.drivenSources.insert(tunClient.drivenSources.end(),
												   channelSettings[ch].sources.begin(),
												   channelSettings[ch].sources.end());
				}
			}
		}
		else
		{
			for(const QString& sourceID : sourcesIDs)
			{
				bool sourceFound = false;

				for(int ch = CHANNEL_1; ch < CHANNELS_COUNT; ch++)
				{
					TuningSource drivenSource = channelSettings[ch].getTuningSource(sourceID);

					if (drivenSource.isValid() == true)
					{
						tunClient.drivenSources.push_back(drivenSource);

						sourceFound = true;
					}
				}

				if (sourceFound == false)
				{
					// Source %1 specified in %2.TuningSourceEquipmentID is not processed by service %3 which the client is connected to.
					//
					log-> errEQP6203(sourceID,
									 tuningClient->equipmentIdTemplate(),
									 software->equipmentIdTemplate());
					result = false;
				}
			}
		}

		clients.push_back(tunClient);
	}

	return result;
}

bool TuningServiceSettingsGetter::fillMatsUsers(const Builder::Context* context)
{
	TEST_PTR_RETURN_FALSE(context);

	matsUsers = context->m_matsUsers.users();

	return true;
}

// -------------------------------------------------------------------------------------
//
// ArchivingServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool ArchivingServiceSettingsGetter::readSettings(const Builder::Context* context,
												  const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT,
											  &clientRequestIP, false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software,
											EquipmentPropNames::CLIENT_REQUEST_NETMASK,
											&clientRequestNetmask,
											false, "", log);
	//

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::APP_DATA_RECEIVING_IP,
											  EquipmentPropNames::APP_DATA_RECEIVING_PORT,
											  &appDataReceivingIP, false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software,
											EquipmentPropNames::APP_DATA_RECEIVING_NETMASK,
											&appDataReceivingNetmask,
											false, "", log);
	//

	result &= DeviceHelper::getIpPortProperty(software,
											  EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
											  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
											  &diagDataReceivingIP, false, "", 0, log);

	result &= DeviceHelper::getIPv4Property(software,
											EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK,
											&diagDataReceivingNetmask,
											false, "", log);

	result &= DeviceHelper::getEnumValueProperty<E::SecurityLevel>(software, EquipmentPropNames::SECURITY_LEVEL, &securityLevel, log);

	//

	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::ARCHIVE_SHORT_TERM_PERIOD, &shortTermArchivePeriod, log);
	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::ARCHIVE_LONG_TERM_PERIOD, &longTermArchivePeriod, log);
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::ARCHIVE_LOCATION, &archiveLocation, log);

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);

	RETURN_IF_FALSE(result);

	result &=checkSettings(software, log);

	return result;
}

bool ArchivingServiceSettingsGetter::checkSettings(const Hardware::Software *software, Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	bool result = true;

	if (archiveLocation.isEmpty() == true)
	{
		log->wrnCFG3031(software->equipmentIdTemplate(), EquipmentPropNames::ARCHIVE_LOCATION);
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// TestClientSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool TestClientSettingsGetter::readSettings(const Builder::Context* context,
											const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// Get CfgService connection

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::CFG_SERVICE_ID1,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&cfgService1_equipmentID,
									&cfgService1_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::ConfigurationService, log);

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::CFG_SERVICE_ID2,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&cfgService2_equipmentID,
									&cfgService2_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::ConfigurationService, log);

	if (cfgService1_equipmentID.isEmpty() == true && cfgService2_equipmentID.isEmpty() == true)
	{
		// Software %1 is not linked to ConfigurationService.
		//
		log->errCFG3029(software->equipmentIdTemplate());
		return false;
	}

	// Get AppDataService connection

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::APP_DATA_SERVICE_ID,
									EquipmentPropNames::APP_DATA_RECEIVING_IP,
									EquipmentPropNames::APP_DATA_RECEIVING_PORT,
									&appDataService_equipmentID,
									&appDataService_appDataReceivingIP,
									true, Socket::IP_NULL,
									PORT_APP_DATA_SERVICE_DATA,
									E::SoftwareType::AppDataService, log);

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::APP_DATA_SERVICE_ID,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&appDataService_equipmentID,
									&appDataService_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::AppDataService, log);

	const Hardware::Software* appDataService = DeviceHelper::getSoftware(equipment, appDataService_equipmentID);

	if (appDataService == nullptr)
	{
		LOG_INTERNAL_ERROR(log);
		return false;
	}

	// Get ArchiveService connection

	result &= getSoftwareConnection(equipment,
									appDataService,
									EquipmentPropNames::ARCH_SERVICE_ID,
									EquipmentPropNames::APP_DATA_RECEIVING_IP,
									EquipmentPropNames::APP_DATA_RECEIVING_PORT,
									&archService_equipmentID,
									&archService_appDataReceivingIP,
									true, Socket::IP_NULL,
									PORT_ARCHIVING_SERVICE_APP_DATA,
									E::SoftwareType::ArchiveService, log);

	result &= getSoftwareConnection(equipment,
									appDataService,
									EquipmentPropNames::ARCH_SERVICE_ID,
									EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
									&archService_equipmentID,
									&archService_diagDataReceivingIP,
									true, Socket::IP_NULL,
									PORT_ARCHIVING_SERVICE_DIAG_DATA,
									E::SoftwareType::ArchiveService, log);

	result &= getSoftwareConnection(equipment,
									appDataService,
									EquipmentPropNames::ARCH_SERVICE_ID,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&archService_equipmentID,
									&archService_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::ArchiveService, log);

	// Get TuningService connection

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::TUNING_SERVICE_ID,
									EquipmentPropNames::TUNING_DATA_IP,
									EquipmentPropNames::TUNING_DATA_PORT,
									&tuningService_equipmentID,
									&tuningService_tuningDataIP,
									true, Socket::IP_NULL,
									PORT_TUNING_SERVICE_DATA,
									E::SoftwareType::TuningService, log);

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::TUNING_SERVICE_ID,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&tuningService_equipmentID,
									&tuningService_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_TUNING_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::TuningService, log);

	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID,
											   &tuningService_tuningSources, log);


	// Get DiagDataService connection

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::DIAG_DATA_SERVICE_ID,
									EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
									&diagDataService_equipmentID,
									&diagDataService_diagDataReceivingIP,
									true, Socket::IP_NULL,
									PORT_DIAG_DATA_SERVICE_DATA,
									E::SoftwareType::DiagDataService, log);

	result &= getSoftwareConnection(equipment,
									software,
									EquipmentPropNames::DIAG_DATA_SERVICE_ID,
									EquipmentPropNames::CLIENT_REQUEST_IP,
									EquipmentPropNames::CLIENT_REQUEST_PORT,
									&diagDataService_equipmentID,
									&diagDataService_clientRequestIP,
									true, Socket::IP_NULL,
									PORT_DIAG_DATA_SERVICE_CLIENT_REQUEST,
									E::SoftwareType::DiagDataService, log);
	return result;
}

// -------------------------------------------------------------------------------------
//
// MetrologySettingsGetter class implementation
//
// -------------------------------------------------------------------------------------


bool MetrologySettingsGetter::readSettings(const Builder::Context* context,
										   const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	appDataServicePropertyIsValid1 = false;
	appDataServicePropertyIsValid2 = false;
	tuningServicePropertyIsValid = false;

	bool result = true;

	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataServiceID1, log);
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataServiceID2, log);

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);
	RETURN_IF_FALSE(result);

	if (appDataServiceID1.isEmpty() == true &&
			appDataServiceID2.isEmpty() == true)
	{
		// Property '%1.%2' is empty.
		//
		log->errCFG3022(software->equipmentId(), EquipmentPropNames::APP_DATA_SERVICE_ID1);
		log->errCFG3022(software->equipmentId(), EquipmentPropNames::APP_DATA_SERVICE_ID2);

		return false;
	}

	if (appDataServiceID1.isEmpty() == false)
	{
		HostAddressPort appDataServiceClientRequestIP1;

		result = getSoftwareConnection(equipment,
									   software,
									   EquipmentPropNames::APP_DATA_SERVICE_ID1,
									   EquipmentPropNames::CLIENT_REQUEST_IP,
									   EquipmentPropNames::CLIENT_REQUEST_PORT,
									   &appDataServiceID1,
									   &appDataServiceClientRequestIP1,
									   true,
									   Socket::IP_NULL,
									   PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
									   E::SoftwareType::AppDataService,
									   log);
		RETURN_IF_FALSE(result);

		appDataServiceIP1 = appDataServiceClientRequestIP1.addressStr();
		appDataServicePort1 = appDataServiceClientRequestIP1.port();

		appDataServicePropertyIsValid1 = true;
	}

	if (appDataServiceID2.isEmpty() == false)
	{
		HostAddressPort appDataServiceClientRequestIP2;

		result = getSoftwareConnection(equipment,
									   software,
									   EquipmentPropNames::APP_DATA_SERVICE_ID2,
									   EquipmentPropNames::CLIENT_REQUEST_IP,
									   EquipmentPropNames::CLIENT_REQUEST_PORT,
									   &appDataServiceID2,
									   &appDataServiceClientRequestIP2,
									   true,
									   Socket::IP_NULL,
									   PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
									   E::SoftwareType::AppDataService,
									   log);
		RETURN_IF_FALSE(result);

		appDataServiceIP2 = appDataServiceClientRequestIP2.addressStr();
		appDataServicePort2 = appDataServiceClientRequestIP2.port();

		appDataServicePropertyIsValid2 = true;
	}

	// TuningService
	//
	HostAddressPort tuningServiceClientRequestIP;

	result = getSoftwareConnection(equipment,
								   software,
								   EquipmentPropNames::TUNING_SERVICE_ID,
								   EquipmentPropNames::CLIENT_REQUEST_IP,
								   EquipmentPropNames::CLIENT_REQUEST_PORT,
								   &tuningServiceID,
								   &tuningServiceClientRequestIP,
								   false,
								   Socket::IP_NULL,
								   PORT_TUNING_SERVICE_CLIENT_REQUEST,
								   E::SoftwareType::TuningService,
								   log);
	RETURN_IF_FALSE(result);

	softwareMetrologyID = software->equipmentIdTemplate();

	tuningServiceIP = tuningServiceClientRequestIP.addressStr();
	tuningServicePort = tuningServiceClientRequestIP.port();

	tuningServicePropertyIsValid = true;

	return	true;
}

// -------------------------------------------------------------------------------------
//
// MonitorSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool MonitorSettingsGetter::readSettings(const Builder::Context* context,
										 const Hardware::Software* software)
{
	clear();

	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();
	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;
	result &= getCfgServiceConnection(equipment, software,
									  &configService1.equipmentId, &configService1.address,
									  &configService2.equipmentId, &configService2.address,
									  log);

	// StartSchemaID
	//
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::START_SCHEMA_ID, &startSchemaId, log);
	RETURN_IF_FALSE(result);

	startSchemaId = startSchemaId.trimmed();

	// SchemaTags
	//
	{
		result = DeviceHelper::getStrProperty(software, EquipmentPropNames::SCHEMA_TAGS, &schemaTags, log);
		RETURN_IF_FALSE(result);

		static const auto re = QRegularExpression("\\W+");
		QStringList schemaTagList = schemaTags.split(re, Qt::SkipEmptyParts);

		for (QString& tag : schemaTagList)
		{
			tag = tag.toLower();
		}

		schemaTags = schemaTagList.join(Separator::SEMICOLON);
	}

	// AppSignalLists
	//
	{
		result = DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_SIGNAL_LISTS, &appSignalLists, log);
		RETURN_IF_FALSE(result);

		static const auto re = QRegularExpression("[\\s;]");
		QStringList appSignalListsList = appSignalLists.split(re, Qt::SkipEmptyParts);
		appSignalLists = appSignalListsList.join(Separator::SEMICOLON);
	}

	// --
	//
	result = readAppDataServiceAndArchiveSettings(context, software);
	RETURN_IF_FALSE(result);

	result = readTuningServiceSettings(context, software);
	RETURN_IF_FALSE(result);

	return result;
}

bool MonitorSettingsGetter::readAppDataServiceAndArchiveSettings(const Builder::Context* context,
																 const Hardware::Software* software)
{
	Builder::IssueLogger* log = context->m_log;
	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// AppDataService settings reading
	//
	QStringList appDataServiceIds;
	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::APP_DATA_SERVICE_IDS, &appDataServiceIds, log);

	if (appDataServiceIds.isEmpty() == true)
	{
		log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS);
		return false;
	}

	// Get all AppDataServices
	//
	std::map<QString, const Hardware::Software*> appDataServices;

	for (const QString& appDataServiceId : appDataServiceIds)
	{
		// ADS->ClientRequestIP, ClientRequestPort
		//
		const Hardware::Software* appDataService = nullptr;

		if (auto appDataServiceDevice = equipment->deviceObject(appDataServiceId);
			appDataServiceDevice == nullptr)
		{
			// Property %1.%2 is linked to undefined software ID %3.
			//
			log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
			result = false;
		}
		else
		{
			if (appDataService = appDataServiceDevice->toSoftware().get();
				appDataService == nullptr)
			{
				// Property %1.%2 is linked to undefined software ID %3.
				//
				log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
				result = false;
			}
			else
			{
				if (appDataService->softwareType() != E::SoftwareType::AppDataService)
				{
					// Property %1.%2 is linked to not compatible software %3.
					//
					log->errCFG3017(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
					result = false;
				}
				else
				{
					appDataServices[appDataServiceId] = appDataService;
				}
			}
		}
	}

	if (result == false || appDataServices.empty() == true)
	{
		return false;
	}

	// Reading AppDataService Settings
	//
	for (const auto&[appDataServiceId, appDataService] : appDataServices)
	{
		// Get AppDataService connection settings
		//
		AppDataServiceSettingsGetter adsSettings;
		result &= adsSettings.readSoftwareSettings(context, appDataService);

		if (result == false)
		{
			return false;
		}

		SoftwareEndpoint::AppDataService ads;
		ads.equipmentId = appDataServiceId;
		ads.address = adsSettings.clientRequestIP;
		ads.realtimeAddress = adsSettings.rtTrendsRequestIP;

		this->appDataServices.push_back(ads);

		// Get ArchiveService connection settings
		//
		QString archiveServiceId;
		HostAddressPort archClientRequestAddress;

		result &= getSoftwareConnection(equipment,
										appDataService,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&archiveServiceId,
										&archClientRequestAddress,
										true,
										Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ArchiveService,
										log);

		if (result == false)
		{
			return false;
		}

		if (archiveServiceId.isEmpty() == true)
		{
			continue;
		}

		SoftwareEndpoint::ArchiveService as;
		as.equipmentId = archiveServiceId;
		as.appDataServiceId = appDataServiceId;
		as.address = archClientRequestAddress;

		this->archiveServices.push_back(as);
	}

	return result;
}

bool MonitorSettingsGetter::readTuningServiceSettings(const Builder::Context* context,
													  const Hardware::Software* software)
{
	Builder::IssueLogger* log = context->m_log;
	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	QStringList tuningServicesIDs;

	bool result = DeviceHelper::getBoolProperty(software, EquipmentPropNames::TUNING_ENABLE, &tuningEnabled, log);

	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::TUNING_SERVICE_ID, &tuningServicesIDs, log);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::TUNING_LOGIN, &tuningLogin, log);

	result &= DeviceHelper::getStrListPropertyAsString(software, EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts, log);

	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout, log);

	tuningServices.clear();

	if (tuningEnabled == false)
	{
		return result;
	}

	for(const QString& tuningServiceID : tuningServicesIDs)
	{
		HostAddressPort tuningServiceClientAddress;

		result &= getSoftwareConnectionBySoftwareID(equipment,
									   software,
									   tuningServiceID,
									   EquipmentPropNames::TUNING_SERVICE_ID,
									   EquipmentPropNames::CLIENT_REQUEST_IP,
									   EquipmentPropNames::CLIENT_REQUEST_PORT,
									   &tuningServiceClientAddress,
									   false,
									   Socket::IP_NULL,
									   PORT_TUNING_SERVICE_CLIENT_REQUEST,
									   E::SoftwareType::TuningService,
									   log);
		BREAK_IF_FALSE(result);

		SoftwareEndpoint::TuningService tsc;

		tsc.equipmentId = tuningServiceID;
		tsc.clientRequestAddress = tuningServiceClientAddress;

		TuningServiceSettingsGetter tsg;

		result &= tsg.readFromDeviceByEquipmentID(context, tuningServiceID, E::SoftwareType::TuningService);

		BREAK_IF_FALSE(result);

		TuningServiceSettings::TuningClient tc = tsg.getTuningClient(software->equipmentIdTemplate());

		if (tc.isValid() == false)
		{
			LOG_INTERNAL_ERROR_MSG(log, QString("Tuning Client %1 is not found in clients list of Tuning Service %2").
								   arg(software->equipmentIdTemplate()).
								   arg(tuningServiceID));
			result = false;
			break;
		}

		tsc.drivenSources = tc.uniqueSourcesIDs();

		tuningServices.push_back(tsc);
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// DiagnosticsSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool DiagnosticsSettingsGetter::readSettings(const Builder::Context* context,
											 const Hardware::Software* software)
{
	clear();

	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;
	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();
	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;
	result &= getCfgServiceConnection(equipment, software,
									  &configService1.equipmentId, &configService1.address,
									  &configService2.equipmentId, &configService2.address,
									  log);

	// StartSchemaID
	//
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::START_SCHEMA_ID, &startSchemaId, log);
	RETURN_IF_FALSE(result);

	startSchemaId = startSchemaId.trimmed();

	// SchemaTags
	//
	result = DeviceHelper::getStrProperty(software, EquipmentPropNames::SCHEMA_TAGS, &schemaTags, log);
	RETURN_IF_FALSE(result);

	static const auto re = QRegularExpression("\\W+");
	QStringList schemaTagList = schemaTags.split(re, Qt::SkipEmptyParts);

	for (QString& tag : schemaTagList)
	{
		tag = tag.toLower();
	}

	schemaTags = schemaTagList.join(Separator::SEMICOLON);

	// --
	//
	result = readDiagDataServiceAndArchiveSettings(context, software);
	RETURN_IF_FALSE(result);

	return result;
}

bool DiagnosticsSettingsGetter::readDiagDataServiceAndArchiveSettings(const Builder::Context* context,
																	  const Hardware::Software* software)
{
	Builder::IssueLogger* log = context->m_log;
	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// DiagDataService settings reading
	//
	QStringList diagDataServiceIds;
	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::DIAG_DATA_SERVICE_IDS, &diagDataServiceIds, log);

	if (diagDataServiceIds.isEmpty() == true)
	{
		log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::DIAG_DATA_SERVICE_IDS);
		return false;
	}

	// Get all DiagDataServices
	//
	std::map<QString, const Hardware::Software*> diagDataServices;

	for (const QString& diagDataServiceId : diagDataServiceIds)
	{
		// DDS->ClientRequestIP, ClientRequestPort
		//
		if (auto diagDataServiceDevice = equipment->deviceObject(diagDataServiceId);
			diagDataServiceDevice == nullptr)
		{
			// Property %1.%2 is linked to undefined software ID %3.
			//
			log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::DIAG_DATA_SERVICE_IDS, diagDataServiceId);
			result = false;
		}
		else
		{
			const Hardware::Software* diagDataService = diagDataServiceDevice->toSoftware().get();

			if (diagDataService == nullptr)
			{
				// Property %1.%2 is linked to undefined software ID %3.
				//
				log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::DIAG_DATA_SERVICE_IDS, diagDataServiceId);
				result = false;
			}
			else
			{
				if (diagDataService->softwareType() != E::SoftwareType::DiagDataService)
				{
					// Property %1.%2 is linked to not compatible software %3.
					//
					log->errCFG3017(software->equipmentIdTemplate(), EquipmentPropNames::DIAG_DATA_SERVICE_IDS, diagDataServiceId);
					result = false;
				}
				else
				{
					diagDataServices[diagDataServiceId] = diagDataService;
				}
			}
		}
	}

	if (result == false || diagDataServices.empty() == true)
	{
		return false;
	}

	// Reading DiagDataService Settings
	//
	for (const auto& [diagDataServiceId, diagDataService] : diagDataServices)
	{
		// Get DiagDataService connection settings
		//
		DiagDataServiceSettingsGetter ddsSettings;

		result &= ddsSettings.readSoftwareSettings(context, diagDataService);
		if (result == false)
		{
			return false;
		}

		SoftwareEndpoint::DiagDataService dds;
		dds.equipmentId = diagDataServiceId;
		dds.address = ddsSettings.clientRequestIP;
		dds.realtimeAddress = ddsSettings.rtTrendsRequestIP;

		this->diagDataServices.push_back(dds);

		// Get ArchiveService connection settings
		//
		QString archiveServiceId;
		HostAddressPort archClientRequestAddress;

		result &= getSoftwareConnection(equipment,
										diagDataService,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&archiveServiceId,
										&archClientRequestAddress,
										true,
										Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ArchiveService,
										log);

		if (result == false)
		{
			return false;
		}

		if (archiveServiceId.isEmpty() == true)
		{
			continue;
		}

		SoftwareEndpoint::ArchiveService as;
		as.equipmentId = archiveServiceId;
		as.appDataServiceId = diagDataServiceId;
		as.address = archClientRequestAddress;

		this->archiveServices.push_back(as);
	}

	return result;
}


// -------------------------------------------------------------------------------------
//
// TuningClientSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool TuningClientSettingsGetter::readSettings(const Builder::Context* context,
											  const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// ConfigurationService connections checking
	//
	result = getCfgServiceConnection(	equipment,
										software,
										&cfgServiceID1, &cfgServiceIP1,
										&cfgServiceID2, &cfgServiceIP2,
										log);
	RETURN_IF_FALSE(result);

	//

	QStringList tuninfServicesIDs;

	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::TUNING_SERVICE_ID, &tuninfServicesIDs, log);

	RETURN_IF_FALSE(result);

	tuningServices.clear();

	for(const QString& tuningServiceID : tuninfServicesIDs)
	{
		HostAddressPort tuningServiceClientAddress;

		result &= getSoftwareConnectionBySoftwareID(equipment,
									   software,
									   tuningServiceID,
									   EquipmentPropNames::TUNING_SERVICE_ID,
									   EquipmentPropNames::CLIENT_REQUEST_IP,
									   EquipmentPropNames::CLIENT_REQUEST_PORT,
									   &tuningServiceClientAddress,
									   false,
									   Socket::IP_NULL,
									   PORT_TUNING_SERVICE_CLIENT_REQUEST,
									   E::SoftwareType::TuningService,
									   log);
		BREAK_IF_FALSE(result);

		SoftwareEndpoint::TuningService tsc;

		tsc.equipmentId = tuningServiceID;
		tsc.clientRequestAddress = tuningServiceClientAddress;

		TuningServiceSettingsGetter tsg;

		result &= tsg.readFromDeviceByEquipmentID(context, tuningServiceID, E::SoftwareType::TuningService);

		BREAK_IF_FALSE(result);

		TuningServiceSettings::TuningClient tc = tsg.getTuningClient(software->equipmentIdTemplate());

		if (tc.isValid() == false)
		{
			LOG_INTERNAL_ERROR_MSG(log, QString("Tuning Client %1 is not found in clients list of Tuning Service %2").
								   arg(software->equipmentIdTemplate()).
								   arg(tuningServiceID));
			result = false;
			break;
		}

		tsc.drivenSources = tc.uniqueSourcesIDs();

		tsc.singleLmControl = tsg.singleLmControl;

		tuningServices.push_back(tsc);
	}

	RETURN_IF_FALSE(result);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::AUTO_APPLAY, &autoApply, log);
	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SHOW_SIGNALS, &showSignals, log);
	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SHOW_SCHEMAS, &showSchemas, log);

	RETURN_IF_FALSE(result);

	//
	// schemasNavigation
	//
	showSchemasList = false;
	showSchemasTabs = false;

	int schemasNavigation = 0;

	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::SCHEMAS_NAVIGATION, &schemasNavigation, log);

	RETURN_IF_FALSE(result);

	switch (schemasNavigation)
	{
	case 0:
		break;
	case 1:
		showSchemasList = true;
		break;
	case 2:
		showSchemasTabs = true;
		break;
	default:
		Q_ASSERT(false);
	}

	//
	// statusFlagFunction
	//

	int value = 0;
	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::STATUS_FLAG_FUNCTION, &value, log);
	if (result == true)
	{
		statusFlagFunction = static_cast<LmStatusFlagMode>(value);
	}

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::TUNING_LOGIN, &tuningLogin, log);

	result &= DeviceHelper::getStrListPropertyAsString(software, EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts, log);

	result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout, log);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation, log);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::FILTER_BY_EQUIPMENT, &filterByEquipment, log);
	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::FILTER_BY_SCHEMA, &filterBySchema, log);

	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::START_SCHEMA_ID, &startSchemaID, log);

	// SchemaTags
	//
	{
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::SCHEMA_TAGS, &schemaTags, log);

		static const auto re = QRegularExpression("\\W+");
		QStringList schemaTagList = schemaTags.split(re, Qt::SkipEmptyParts);

		schemaTags = schemaTagList.join(Separator::SEMICOLON);
	}

	// AppSignalLists
	//
	{
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_SIGNAL_LISTS, &appSignalLists, log);

		static const auto re = QRegularExpression("[\\s;]");
		QStringList appSignalListsList = appSignalLists.split(re, Qt::SkipEmptyParts);
		appSignalLists = appSignalListsList.join(Separator::SEMICOLON);
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// TestSuiteSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool TestSuiteSettingsGetter::readSettings(const Builder::Context* context,
										   const Hardware::Software* software)
{
	clear();

	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	TEST_PTR_LOG_RETURN_FALSE(equipment, log);

	bool result = true;

	result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
									  &cfgServiceID2, &cfgServiceIP2, log);


	result &= readAppDataServiceAndArchiveSettings(context, software);

	RETURN_IF_FALSE(result);

	result &= readTuningServiceSettings(context, software);

	RETURN_IF_FALSE(result);

	result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::TESTING_LOGIN, &login, log);
	
	result &= DeviceHelper::getStrListPropertyAsString(software, EquipmentPropNames::TESTING_USER_ACCOUNTS, &userAccounts, log);

	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::TESTING_PLANT, &plant, log);
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::TESTING_UNIT, &unit, log);
	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::TESTING_SYSTEM, &system, log);

	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::TESTING_SCRIPTTAGS, &scriptTags, log);

	return result;
}

bool TestSuiteSettingsGetter::readAppDataServiceAndArchiveSettings(const Builder::Context* context,
																   const Hardware::Software* software)
{
	Builder::IssueLogger* log = context->m_log;
	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// AppDataService settings reading
	//
	QStringList appDataServiceIds;
	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::APP_DATA_SERVICE_IDS, &appDataServiceIds, log);

	// Maybe we will have tests without AppDataService
	//
//	if (appDataServiceIds.isEmpty() == true)
//	{
//		log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS);
//		return false;
//	}

	// Get all AppDataServices
	//
	std::map<QString, const Hardware::Software*> appDataServices;

	for (const QString& appDataServiceId : appDataServiceIds)
	{
		// ADS->ClientRequestIP, ClientRequestPort
		//
		const Hardware::Software* appDataService = nullptr;

		if (auto appDataServiceDevice = equipment->deviceObject(appDataServiceId);
			appDataServiceDevice == nullptr)
		{
			// Property %1.%2 is linked to undefined software ID %3.
			//
			log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
			result = false;
		}
		else
		{
			if (appDataService = appDataServiceDevice->toSoftware().get();
				appDataService == nullptr)
			{
				// Property %1.%2 is linked to undefined software ID %3.
				//
				log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
				result = false;
			}
			else
			{
				if (appDataService->softwareType() != E::SoftwareType::AppDataService)
				{
					// Property %1.%2 is linked to not compatible software %3.
					//
					log->errCFG3017(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_IDS, appDataServiceId);
					result = false;
				}
				else
				{
					appDataServices[appDataServiceId] = appDataService;
				}
			}
		}
	}

	if (result == false)
	{
		return false;
	}

	// Reading AppDataService Settings
	//
	for (const auto&[appDataServiceId, appDataService] : appDataServices)
	{
		// Get AppDataService connection settings
		//
		AppDataServiceSettingsGetter adsSettings;
		result &= adsSettings.readSoftwareSettings(context, appDataService);

		if (result == false)
		{
			return false;
		}

		SoftwareEndpoint::AppDataService ads;
		ads.equipmentId = appDataServiceId;
		ads.address = adsSettings.clientRequestIP;
		ads.realtimeAddress = adsSettings.rtTrendsRequestIP;

		this->appDataServices.push_back(ads);
	}

	return result;
}

bool TestSuiteSettingsGetter::readTuningServiceSettings(const Builder::Context* context,
														const Hardware::Software* software)
{
	Builder::IssueLogger* log = context->m_log;
	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	QStringList tuningServicesIDs;

	bool result = DeviceHelper::getBoolProperty(software, EquipmentPropNames::TUNING_ENABLE, &tuningEnabled, log);

	result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::TUNING_SERVICE_ID, &tuningServicesIDs, log);

	tuningServices.clear();

	if (tuningEnabled == false)
	{
		return result;
	}

	for(const QString& tuningServiceID : tuningServicesIDs)
	{
		HostAddressPort clientRequestAddress;

		result &= getSoftwareConnectionBySoftwareID(equipment,
													software,
													tuningServiceID,
													EquipmentPropNames::TUNING_SERVICE_ID,
													EquipmentPropNames::CLIENT_REQUEST_IP,
													EquipmentPropNames::CLIENT_REQUEST_PORT,
													&clientRequestAddress,
													false,
													Socket::IP_NULL,
													PORT_TUNING_SERVICE_CLIENT_REQUEST,
													E::SoftwareType::TuningService,
													log);
		BREAK_IF_FALSE(result);

		SoftwareEndpoint::TuningService tsc;

		tsc.equipmentId = tuningServiceID;
		tsc.clientRequestAddress = clientRequestAddress;

		TuningServiceSettingsGetter tsg;

		result &= tsg.readFromDeviceByEquipmentID(context, tuningServiceID, E::SoftwareType::TuningService);

		BREAK_IF_FALSE(result);

		TuningServiceSettings::TuningClient tc = tsg.getTuningClient(software->equipmentIdTemplate());

		if (tc.isValid() == false)
		{
			LOG_INTERNAL_ERROR_MSG(log, QString("Tuning Client %1 is not found in clients list of Tuning Service %2").
								   arg(software->equipmentIdTemplate()).
								   arg(tuningServiceID));
			result = false;
			break;
		}

		tsc.drivenSources = tc.uniqueSourcesIDs();

		tuningServices.push_back(tsc);
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// GatewayServiceSettingsGetter class implementation
//
// -------------------------------------------------------------------------------------

bool GatewayServiceSettingsGetter::readSettings(const Builder::Context* context,
												const Hardware::Software* software)
{
	TEST_PTR_RETURN_FALSE(context);

	Builder::IssueLogger* log = context->m_log;

	TEST_PTR_RETURN_FALSE(log);
	TEST_PTR_LOG_RETURN_FALSE(software, log);

	const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

	bool result = true;

	// Get ConfigurationService connections
	//
	std::vector<SoftwareEndpoint::ConfigService> cfgSrvConns;

	result &= getSoftwareConnectionsBySoftwareIDs<SoftwareEndpoint::ConfigService>(
										equipment, software,
										EquipmentPropNames::CFG_SERVICE_IDS, 2, false,
										E::SoftwareType::ConfigurationService,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&cfgSrvConns, log);
	cfgService1 = cfgSrvConns[0];
	cfgService2 = cfgSrvConns[1];

	// Get AppDataService connections
	//
	std::vector<SoftwareEndpoint::AppDataService> appDataSrvConns;

	result &= getSoftwareConnectionsBySoftwareIDs<SoftwareEndpoint::AppDataService>(
										equipment, software,
										EquipmentPropNames::APP_DATA_SERVICE_IDS, 2, false,
										E::SoftwareType::AppDataService,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&appDataSrvConns, log);
	appDataService1 = appDataSrvConns[0];
	appDataService2 = appDataSrvConns[1];

	result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::GATEWAY_DESCRIPTION,
										   &gatewayDescription, log);
	return result;
}
