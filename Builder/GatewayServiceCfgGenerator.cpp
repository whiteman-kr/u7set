#include "GatewayServiceCfgGenerator.h"
#include "AppDataServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"
#include "../GatewayLib/AdsGateway.h"
#include <GatewayClientLib/AdsGwProtocol.hpp>

namespace Builder
{

	GatewayServiceCfgGenerator::GatewayServiceCfgGenerator(Context* context, Hardware::Software* software)	:
		SoftwareCfgGenerator(context, software)
	{
	}

	bool GatewayServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		GatewayServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = true;

		result &= m_settingsSet.addProfile<GatewayServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool GatewayServiceCfgGenerator::generateConfigurationStep1()
	{
		return true;
	}

	bool GatewayServiceCfgGenerator::generateConfigurationStep2()
	{
		if (m_software == nullptr ||
			m_software->softwareType() != E::SoftwareType::GatewayService ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			Q_ASSERT(m_software);
			Q_ASSERT(m_software->softwareType() == E::SoftwareType::GatewayService);
			Q_ASSERT(m_equipment);
			Q_ASSERT(m_cfgXml);
			Q_ASSERT(m_buildResultWriter);
			return false;
		}

		IssueLogger* log = m_buildResultWriter->log();

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		std::shared_ptr<const GatewayServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<GatewayServiceSettings>();

		LOG_MESSAGE(log, QString("Parsing of %1 gateway description started...").arg(equipmentID()));

		m_gateways = std::make_shared<Gateway::Gateways>();

		m_parser = std::make_shared<Gateway::Parser>(m_context, m_gateways);

		result = m_parser->parse(settings->gatewayDescription);

		int errCount = m_parser->errorCount();
		int wrnCount = m_parser->warningCount();

		BuildFile* buildFile = nullptr;

		if (errCount == 0)
		{
			for(const Gateway::GatewayShared& gw : *m_gateways)
			{
				TEST_PTR_CONTINUE(gw);

				result &= doGatewaySpecificProcessing(gw);

				const auto& files = gw->files();

				for(const Gateway::File& file : files)
				{
					buildFile = m_buildResultWriter->addFile(
						softwareCfgSubdir() + Separator::DIR + file.gatewayID(),
						file.fileName(), file.fileData());

					if (buildFile == nullptr)
					{
						errCount++;
						result = false;
					}
				}
			}

			QString xmlStr;
			XmlWriteHelper xml(&xmlStr);

			m_gateways->writeToXml(xml);

			buildFile = m_buildResultWriter->addFile(
				softwareCfgSubdir(),
				File::GATEWAY_DESCRIPTION_XML,
				CfgFileId::GATEWAY_DESCRIPTION,
				QString(),
				xmlStr);

			if (buildFile == nullptr)
			{
				errCount++;
				result = false;
			}
			else
			{
				m_cfgXml->addLinkToFile(buildFile);
			}
		}

		buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
												 File::GATEWAY_DESCRIPTION_TXT,
												 settings->gatewayDescription);
		if (buildFile == nullptr)
		{
			errCount++;
			result = false;
		}
		else
		{
			m_cfgXml->addLinkToFile(buildFile);
		}

		buildFile = m_buildResultWriter->getBuildFileByID(Directory::COMMON, CfgFileId::APP_SIGNAL_SET);

		if (buildFile == nullptr)
		{
			errCount++;
			result = false;
		}
		else
		{
			m_cfgXml->addLinkToFile(buildFile);
		}

		QString resultStr = QString("parsing of %1 gateway description finished with %2 errors, %3 warnings").
							arg(equipmentID()).arg(errCount).arg(wrnCount);
		if (errCount > 0)
		{
			m_log->errCFG3051(resultStr);
		}
		else
		{
			if (wrnCount > 0)
			{
				m_log->wrnCFG3052(resultStr);
			}
			else
			{
				resultStr = resultStr.mid(0, 1).toUpper() + resultStr.mid(1);
				LOG_MESSAGE(m_log, resultStr);
			}
		}

		return result;
	}

	bool GatewayServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														const GatewayServiceSettings& settings,
														E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgService1.address,
										   settings.cfgService2.address, os, true);

		if (cmdLine.isEmpty() == true)
		{
			return false;
		}

		content += cmdLine;

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);
		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool GatewayServiceCfgGenerator::doGatewaySpecificProcessing(const Gateway::GatewayShared& gw)
	{
		bool result = true;

		switch(gw->gatewayType())
		{
		case Gateway::E::GatewayType::IVS_Impulse:
			result &= ivsImpulseGatewayProcessing(gw);
			break;

		case Gateway::E::GatewayType::ModbusSlave:
			result &= modbusSlaveGatewayProcessing(gw);
			break;

		case Gateway::E::GatewayType::AdsGateway:
			result &= adsGatewayProcessing(gw);
			break;

		case Gateway::E::GatewayType::TuningGateway:
			result &= tuningGatewayProcessing(gw);
			break;

		default: ;
		}

		return result;
	}

	bool GatewayServiceCfgGenerator::ivsImpulseGatewayProcessing(const Gateway::GatewayShared& gw)
	{
		bool result = true;

		result = checkConnection(gw, E::AppDataService);

		return result;
	}

	bool GatewayServiceCfgGenerator::modbusSlaveGatewayProcessing(const Gateway::GatewayShared& gw)
	{
		bool result = true;

		result = checkConnection(gw, E::AppDataService);

		return result;
	}

	namespace AGL = GatewayClientLib;

	bool GatewayServiceCfgGenerator::adsGatewayProcessing(const Gateway::GatewayShared& gw)
	{
		Gateway::AdsGatewayShared adsGw = std::dynamic_pointer_cast<Gateway::AdsGateway>(gw);

		if (adsGw == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		result = checkConnection(gw, E::AppDataService);

		RETURN_IF_FALSE(result);

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const GatewayServiceSettings> settings  =
						m_settingsSet.getSettingsProfile<GatewayServiceSettings>(profile);

			TEST_PTR_CONTINUE(settings);

			QStringList controllerIDs;

			if (settings->appDataService1.equipmentId.isEmpty() == false)
			{
				controllerIDs.append(settings->appDataService1.equipmentId);
			}

			if (settings->appDataService2.equipmentId.isEmpty() == false)
			{
				controllerIDs.append(settings->appDataService2.equipmentId);
			}

			std::set<Hash> acquiredSignals;

			for(const QString& controllerID : controllerIDs)
			{
				std::shared_ptr<Hardware::DeviceObject> device = m_equipment->deviceObject(controllerID);

				if (device == nullptr ||
					device->deviceType() != Hardware::DeviceType::Controller)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				device = device->parent();

				if (device == nullptr ||
					device->deviceType() != Hardware::DeviceType::Software ||
					device->toSoftware() == nullptr ||
					device->toSoftware()->softwareType() != E::SoftwareType::AppDataService)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				QString appDataSrvID = device->equipmentIdTemplate();

				auto it = m_context->m_swCfgGens.find(appDataSrvID);

				if (it == m_context->m_swCfgGens.end())
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				std::shared_ptr<AppDataServiceCfgGenerator> adsCfgGen =
						std::dynamic_pointer_cast<AppDataServiceCfgGenerator>(it->second);

				if (adsCfgGen == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				const std::set<Hash> appDataSrvAcquiredSignals = adsCfgGen->acquiredAppSignals();

				acquiredSignals.insert(appDataSrvAcquiredSignals.begin(),
									   appDataSrvAcquiredSignals.end());
			}

			if (result == false)
			{
				break;
			}

			QStringList appSignalIDs;

			for(Hash hash : acquiredSignals)
			{
				const AppSignal* appSignal = m_signalSet->getSignalByHash(hash);

				TEST_PTR_CONTINUE(appSignal);

				//

				const QString& appSignalID = appSignal->appSignalID();

				result &= checkStrLen(appSignalID, appSignal->appSignalID(), AGL::STRING_LENGTH_128, QStringLiteral("appSignalID"));
				result &= checkStrLen(appSignalID, appSignal->customAppSignalID(), AGL::STRING_LENGTH_128, QStringLiteral("customAppSignalID"));
				result &= checkStrLen(appSignalID, appSignal->caption(), AGL::STRING_LENGTH_256, QStringLiteral("caption"));
				result &= checkStrLen(appSignalID, appSignal->equipmentID(), AGL::STRING_LENGTH_128, QStringLiteral("equipmentID"));
				result &= checkStrLen(appSignalID, appSignal->lmEquipmentID(), AGL::STRING_LENGTH_128, QStringLiteral("lmEquipmentID"));
				result &= checkStrLen(appSignalID, appSignal->unit(), AGL::STRING_LENGTH_128, QStringLiteral("unit"));
				result &= checkStrLen(appSignalID, appSignal->tagsStr(), AGL::STRING_LENGTH_256, QStringLiteral("tags"));

				//

				appSignalIDs.append(appSignal->appSignalID());
			}

			adsGw->appendSignalList(profile, appSignalIDs);
		}

		return result;
	}

	bool GatewayServiceCfgGenerator::tuningGatewayProcessing(const Gateway::GatewayShared& gw)
	{
		bool result = true;

		result = checkConnection(gw, E::TuningService);

/*		Gateway::AdsGatewayShared adsGw = std::dynamic_pointer_cast<Gateway::AdsGateway>(gw);

		if (adsGw == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const GatewayServiceSettings> settings  =
				m_settingsSet.getSettingsProfile<GatewayServiceSettings>(profile);

			TEST_PTR_CONTINUE(settings);

			QStringList controllerIDs;

			if (settings->appDataService1.equipmentId.isEmpty() == false)
			{
				controllerIDs.append(settings->appDataService1.equipmentId);
			}

			if (settings->appDataService2.equipmentId.isEmpty() == false)
			{
				controllerIDs.append(settings->appDataService2.equipmentId);
			}

			std::set<Hash> acquiredSignals;

			for(const QString& controllerID : controllerIDs)
			{
				std::shared_ptr<Hardware::DeviceObject> device = m_equipment->deviceObject(controllerID);

				if (device == nullptr ||
					device->deviceType() != Hardware::DeviceType::Controller)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				device = device->parent();

				if (device == nullptr ||
					device->deviceType() != Hardware::DeviceType::Software ||
					device->toSoftware() == nullptr ||
					device->toSoftware()->softwareType() != E::SoftwareType::AppDataService)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				QString appDataSrvID = device->equipmentIdTemplate();

				auto it = m_context->m_swCfgGens.find(appDataSrvID);

				if (it == m_context->m_swCfgGens.end())
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				std::shared_ptr<AppDataServiceCfgGenerator> adsCfgGen =
					std::dynamic_pointer_cast<AppDataServiceCfgGenerator>(it->second);

				if (adsCfgGen == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					break;
				}

				const std::set<Hash> appDataSrvAcquiredSignals = adsCfgGen->acquiredAppSignals();

				acquiredSignals.insert(appDataSrvAcquiredSignals.begin(),
									   appDataSrvAcquiredSignals.end());
			}

			if (result == false)
			{
				break;
			}

			QStringList appSignalIDs;

			for(Hash hash : acquiredSignals)
			{
				const AppSignal* appSignal = m_signalSet->getSignalByHash(hash);

				TEST_PTR_CONTINUE(appSignal);

				//

				const QString& appSignalID = appSignal->appSignalID();

				result &= checkStrLen(appSignalID, appSignal->appSignalID(), AGL::STRING_LENGTH_128, QStringLiteral("appSignalID"));
				result &= checkStrLen(appSignalID, appSignal->customAppSignalID(), AGL::STRING_LENGTH_128, QStringLiteral("customAppSignalID"));
				result &= checkStrLen(appSignalID, appSignal->caption(), AGL::STRING_LENGTH_256, QStringLiteral("caption"));
				result &= checkStrLen(appSignalID, appSignal->equipmentID(), AGL::STRING_LENGTH_128, QStringLiteral("equipmentID"));
				result &= checkStrLen(appSignalID, appSignal->lmEquipmentID(), AGL::STRING_LENGTH_128, QStringLiteral("lmEquipmentID"));
				result &= checkStrLen(appSignalID, appSignal->unit(), AGL::STRING_LENGTH_128, QStringLiteral("unit"));
				result &= checkStrLen(appSignalID, appSignal->tagsStr(), AGL::STRING_LENGTH_256, QStringLiteral("tags"));

				//

				appSignalIDs.append(appSignal->appSignalID());
			}

			adsGw->appendSignalList(profile, appSignalIDs);
		}*/

		return result;
	}

	bool GatewayServiceCfgGenerator::checkConnection(const Gateway::GatewayShared& gw, E::SoftwareType swType)
	{
		TEST_PTR_RETURN_FALSE(gw);

		bool result = true;

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const GatewayServiceSettings> settings  =
				m_settingsSet.getSettingsProfile<GatewayServiceSettings>(profile);

			TEST_PTR_CONTINUE(settings);

			bool res = true;

			switch(swType)
			{
			case E::SoftwareType::AppDataService:
				res = !(settings->appDataService1.equipmentId.isEmpty() &&
						settings->appDataService2.equipmentId.isEmpty());
				break;

			case E::SoftwareType::TuningService:
				res = !(settings->tuningService1.equipmentId.isEmpty() &&
						settings->tuningService2.equipmentId.isEmpty());
				break;

			default:
				Q_ASSERT(false);
				res = false;
			}

			if (res == false)
			{
				// Gateway service %1 must be connected to %2 for Gateway %3 (type %4) profile %5
				//
				m_log->errCFG3056(m_software->equipmentIdTemplate(), E::valueToString(swType),
								  gw->gatewayID(), E::valueToString(gw->gatewayType()), profile);
			}

			result &= res;
		}

		return result;
	}

	bool GatewayServiceCfgGenerator::checkStrLen(const QString& appSignalID, const QString& str, size_t len, const QString& propName)
	{
		Q_ASSERT(len > 0);

		if (static_cast<size_t>(str.toUtf8().size()) > len - 1)
		{
			// Property %1.%2 exceed length of %3 bytes
			//
			m_log->errCFG3105(appSignalID, propName, len - 1);

			return false;
		}

		return true;
	}
}
