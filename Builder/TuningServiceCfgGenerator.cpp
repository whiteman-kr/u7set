#include "TuningServiceCfgGenerator.h"
#include "Context.h"
#include "Builder.h"
#include "../TuningService/TuningSource.h"
#include "../lib/SoftwareSettingsGetter.h"

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

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<TuningServiceSettings>(profile, settingsGetter);

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
			if (writeTuningSources() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool TuningServiceCfgGenerator::writeTuningSources()
	{
		QStringList profiles = m_settingsSet.getSettingsProfiles();

		QVector<Tuning::TuningSource> tuningSources;

		bool result = true;

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const TuningServiceSettings> settings =
					m_settingsSet.getSettingsProfile<TuningServiceSettings>(profile);

			TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

			std::set<QString> alreadyAppendSources;

			for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
			{
				const TuningServiceSettings::ChannelSettings& ch = settings->channelSettings[channel];

				for(const TuningServiceSettings::TuningSource& tunSrc : ch.sources)
				{
					if (alreadyAppendSources.contains(tunSrc.lmEquipmentID) == true)
					{
						continue;
					}

					alreadyAppendSources.insert(tunSrc.lmEquipmentID);

					std::shared_ptr<Hardware::DeviceObject> device = m_equipment->deviceObject(tunSrc.lmEquipmentID);

					if (device == nullptr)
					{
						// Equipment object %1 is not found (Settings profile - %2).
						//
						m_log->errCFG3044(tunSrc.lmEquipmentID, profile);
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
		}

		QByteArray fileData;

		result &= DataSourcesXML<Tuning::TuningSource>::writeToXml(tuningSources, &fileData);

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
