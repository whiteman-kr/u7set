#include "DiagDataServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../lib/DataSource.h"

namespace Builder
{
	DiagDataServiceCfgGenerator::DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	DiagDataServiceCfgGenerator::~DiagDataServiceCfgGenerator()
	{
	}

	bool DiagDataServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		DiagDataServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<DiagDataServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool DiagDataServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeDiagDataSourcesXml() == false) break;
//			if (writeAppSignalsXml() == false) break;				// AppSignals.xml for AZPZ
//			if (writeAppSignalsExtXml() == false) break;			// AppSignalsExt.xml as extra debug info
//			if (writeAcquiredAppSignalsFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool DiagDataServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														 const DiagDataServiceSettings& settings,
														 E::OS os)
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

	bool DiagDataServiceCfgGenerator::writeDiagDataSourcesXml()
	{
		bool result = true;

		m_acquiredDiagSignals.clear();

		QVector<DataSource> dataSources;

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const DiagDataServiceSettings> settings =
					m_settingsSet.getSettingsProfile<DiagDataServiceSettings>(profile);

			TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

			quint32 receivingNetmask = settings->appDataReceivingNetmask.toIPv4Address();

			quint32 receivingSubnet = settings->appDataReceivingIP.address32() & receivingNetmask;

			for(Hardware::DeviceModule* lm : m_context->m_lmModules)
			{
				if (lm == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				DataSource ds;

				ds.setProfile(profile);

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, E::LanControllerType::DiagData,
																			m_context, &ds);

				ds.lanControllersInfo().filterLansByDiagDataServiceID(m_software->equipmentIdTemplate());

				int connectedAdaptersCount = 0;

				for(const LanControllerInfo& lan : ds.lanControllersInfo()())
				{
					if (lan.diagDataEnable == false || lan.diagDataServiceID != m_software->equipmentIdTemplate())
					{
						continue;
					}

					if (connectedAdaptersCount > 0)
					{
						// Several ethernet adapters of LM %1 are connected to same AppDataService %2.
						//
						m_log->errCFG3030(lm->equipmentIdTemplate(), m_software->equipmentIdTemplate());
						result = false;
						continue;
					}

					if ((QHostAddress(lan.appDataIP).toIPv4Address() & receivingNetmask) != receivingSubnet)
					{
						// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
						//
						m_log->errCFG3043(lan.appDataIP,
										  lan.equipmentID,
										  settings->appDataReceivingIP.addressStr(),
										  equipmentID());
						result = false;
						continue;
					}

					connectedAdaptersCount++;

					result &= findAppDataSourceAcquiredSignals(ds);	// inside fills m_associatedAppSignals also

					dataSources.append(ds);
				}
			}
		}

		RETURN_IF_FALSE(result)

		//

		QByteArray fileData;
		result &= DataSourcesXML<DataSource>::writeToXml(dataSources, &fileData);

		RETURN_IF_FALSE(result)

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::APP_DATA_SOURCES_XML, CfgFileId::APP_DATA_SOURCES, "", fileData);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return result;
	}


}
