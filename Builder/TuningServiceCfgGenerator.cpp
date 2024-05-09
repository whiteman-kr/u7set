#include "TuningServiceCfgGenerator.h"
#include "Context.h"
#include "SoftwareSettingsGetter.h"

#include "../TuningService/TuningSource.h"

#include <HardwareLib/DeviceModule.h>


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(Context* context,
														 Hardware::Software* software) :
		SoftwareCfgGenerator(context, software),
		m_tuningDataStorage(context->m_tuningDataStorage.get())
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}

	bool TuningServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TuningServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = true;

		const TuningServiceSettings& settings = settingsGetter;

		for(int ch = CHANNEL_1; ch < settings.channelCount; ch++)
		{
			HostAddressPort tuningSimIP = settings.channelSettings[ch].tuningSimIP;

			QString thisObjID = settings.channelSettings[ch].serviceControllerEquipmentID;

			QString str = QString("%1_%2").arg(profile).arg(tuningSimIP.addressPortStr());

			auto it = m_tuningSimIpPorts.find(str);

			if (it == m_tuningSimIpPorts.end())
			{
				m_tuningSimIpPorts.insert({str, thisObjID});
			}
			else
			{
				// TuningSimIP %1 is not unique in objects %2 and %3 (profile %4)
				//
				m_log->errCFG3104(tuningSimIP.addressPortStr(), it->second, thisObjID, profile);

				result = false;
			}
		}

		RETURN_IF_FALSE(result);

		result &= m_settingsSet.addProfile<TuningServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool TuningServiceCfgGenerator::generateConfigurationStep1()
	{
		if (m_tuningDataStorage == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = false;

		do
		{
			if (writeTuningSourcesXml() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool TuningServiceCfgGenerator::writeTuningSourcesXml()
	{
		QStringList profiles = m_settingsSet.getSettingsProfiles();

		QVector<Tuning::TuningSource> tuningSources;

		bool result = true;

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const TuningServiceSettings> settings =
					m_settingsSet.getSettingsProfile<TuningServiceSettings>(profile);

			TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

			// TunableLmEquipmentID => set of LinkID(s) to TuningService
			//
			// Link to single-channel TuningService is TuningService.EquipmentID
			// Links to two-channel TuningService are TuninService.ChannelController.EquipmentIDs
			//
			std::map<QString, std::set<QString>> lmToServiceLinksID;

			for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
			{
				const TuningServiceSettings::ChannelSettings& ch = settings->channelSettings[channel];

				for(const TuningServiceSettings::TuningSource& tunSrc : ch.sources)
				{
					auto srcIt = lmToServiceLinksID.find(tunSrc.lmEquipmentID);

					if (srcIt == lmToServiceLinksID.end())
					{
						auto p = lmToServiceLinksID.insert({tunSrc.lmEquipmentID, std::set<QString>()});
						srcIt = p.first;
					}

					std::set<QString>& linksSet = srcIt->second;

					if (settings->isTwoChannelTuningService == false)
					{
						// To single-channel TuningService LM must links via TuningService.EquipmentID
						//
						Q_ASSERT(m_software->equipmentIdTemplate() == settings->equipmentID);

						linksSet.insert(settings->equipmentID);
					}
					else
					{
						// To two-channel TuningService LM must links via TuningService.ChannelController.EquipmentID
						//
						linksSet.insert(ch.serviceControllerEquipmentID);
					}
				}
			}

			for(const auto& p : lmToServiceLinksID)
			{
				const QString& tunableLmID = p.first;
				const std::set<QString>& linksToTuningService = p.second;

				std::shared_ptr<Hardware::DeviceObject> device = m_equipment->deviceObject(tunableLmID);

				if (device == nullptr)
				{
					// Equipment object %1 is not found (Settings profile - %2).
					//
					m_log->errCFG3044(tunableLmID, profile);
					result = false;
					continue;
				}

				Hardware::DeviceModule* lm = dynamic_cast<Hardware::DeviceModule*>(device.get());

				if (lm == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				Tuning::TuningSource ts;

				ts.setProfile(profile);

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm,
														E::LanControllerType::Tuning,
														m_context, &ts);

				if (result == false)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("Error getLmPropertiesFromDevice %1").arg(tunableLmID));
					continue;
				}

				ts.lanControllersInfo().filterLansByTuningServiceLinkIDs(linksToTuningService);

				if (ts.lanControllersInfo()().size() == 0)
				{
					continue;
				}

				Tuning::TuningDataShared tuningData = m_context->m_tuningDataStorage->getTuningData(lm->equipmentId());

				if(tuningData != nullptr)
				{
					ts.setTuningData(tuningData);
				}
				else
				{
					// Tuning data is not found for module %1
					//
					m_log->errALC5197(lm->equipmentIdTemplate());
					result = false;
				}

				tuningSources.push_back(ts);
			}
		}

		QByteArray fileData;

		result &= OnlineLib::DataSourcesXML<Tuning::TuningSource>::writeToXml(tuningSources, &fileData);

		RETURN_IF_FALSE(result)

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::TUNING_SOURCES_XML, CfgFileId::TUNING_SOURCES, "", fileData);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return result;
	}

	bool TuningServiceCfgGenerator::writeRunScriptFile(const QString& profile, const TuningServiceSettings& settings, E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgServiceIP1, settings.cfgServiceIP2, os, true);

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
}
