#include "AppDataServiceCfgGenerator.h"
#include "Builder.h"

#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../lib/DeviceHelper.h"
#include "../lib/DataSource.h"
#include "../lib/SoftwareSettingsGetter.h"

class DataSource;

namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(Context* context,
														   Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
		Q_ASSERT(context != nullptr);
	}

	AppDataServiceCfgGenerator::~AppDataServiceCfgGenerator()
	{
	}

	bool AppDataServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		AppDataServiceSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		bool result =  m_settingsSet.addProfile<AppDataServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool AppDataServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeAppDataSourcesXml() == false) break;
			if (writeAppSignalsXml() == false) break;
			if (addLinkToAppSignalsFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppDataSourcesXml()
	{
		bool result = true;

		m_associatedAppSignals.clear();

		QVector<DataSource> dataSources;

		std::shared_ptr<const AppDataServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<AppDataServiceSettings>();

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

			result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, E::LanControllerType::AppData,
																		m_context, &ds);
			int connectedAdaptersCount = 0;

			for(const LanControllerInfo& lan : ds.lanControllersInfo()())
			{
				if (lan.appDataEnable == false || lan.appDataServiceID != m_software->equipmentIdTemplate())
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

				result &= findAppDataSourceAssociatedSignals(ds);	// inside fills m_associatedAppSignals also

				dataSources.append(ds);
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

	bool AppDataServiceCfgGenerator::writeAppSignalsXml()
	{
		if (m_context->generateAppSignalsXml() == false)
		{
			return true;
		}

		QByteArray azpzData;
		XmlWriteHelper azpzXml(&azpzData);

		azpzXml.setAutoFormatting(true);
		azpzXml.writeStartDocument();
		azpzXml.writeStartElement(XmlElement::APP_SIGNALS);
		azpzXml.writeIntAttribute(XmlAttribute::BUILD_ID, m_buildResultWriter->buildInfo().id);
		azpzXml.writeStartElement(XmlElement::SIGNALS);
		azpzXml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_associatedAppSignals.size()));

		QByteArray extData;
		XmlWriteHelper extXml(&extData);

		extXml.setAutoFormatting(true);
		extXml.writeStartDocument();
		extXml.writeStartElement(XmlElement::APP_SIGNALS);
		extXml.writeIntAttribute(XmlAttribute::BUILD_ID, m_buildResultWriter->buildInfo().id);
		extXml.writeStartElement(XmlElement::SIGNALS);
		extXml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_associatedAppSignals.size()));

		int signalCount = m_signalSet->count();
		int writtenSignalsCount = 0;

		for(int i = 0; i < signalCount; i++)
		{
			AppSignal& signal = (*m_signalSet)[i];

			if (m_associatedAppSignals.contains(signal.appSignalID()) == false)
			{
				continue;
			}

			signal.writeToAzpzXml(azpzXml);
			signal.writeToXml(extXml);

			writtenSignalsCount++;
		}

		if (writtenSignalsCount != m_associatedAppSignals.size())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
		}

		azpzXml.writeEndElement();	// </Signals>
		azpzXml.writeEndElement();	// </AppSignals>
		azpzXml.writeEndDocument();

		extXml.writeEndElement();	// </Signals>
		extXml.writeEndElement();	// </AppSignals>
		extXml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
															File::APP_SIGNALS_XML,
															CfgFileId::APP_SIGNALS, "", azpzData);

		if (buildFile == nullptr)
		{
			return false;
		}

		buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
													File::APP_SIGNALS_EXT_XML,
													CfgFileId::APP_SIGNALS_EXT, "", extData);

		if (buildFile == nullptr)
		{
			return false;
		}

		return true;
	}

	bool AppDataServiceCfgGenerator::addLinkToAppSignalsFile()
	{
		// add link to signals set file
		//
		// After task RPCT-2170 resolving (separate signalset files for each AppDataService)
		// this link should be removed !!!

		BuildFile* buildFile = m_buildResultWriter->getBuildFileByID(Directory::COMMON, CfgFileId::APP_SIGNAL_SET);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}

	bool AppDataServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														const AppDataServiceSettings& settings,
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

	bool AppDataServiceCfgGenerator::findAppDataSourceAssociatedSignals(DataSource& appDataSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(appDataSource.moduleEquipmentID()).get();

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const Hardware::DeviceChassis* dataSourceChassis = lm->getParentChassis();

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			const AppSignal& appSignal =  (*m_signalSet)[i];

			QString appSignalEquipmentID = appSignal.equipmentID();

			if (appSignalEquipmentID.isEmpty())
			{
				continue;
			}

			Hardware::DeviceObject* device = m_equipment->deviceObject(appSignalEquipmentID).get();

			if (device == nullptr)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrefix::NotDefined, QString("Signal '%1' bound with an unknown device '%2'").
					arg(appSignal.appSignalID()).arg(appSignalEquipmentID));
				continue;
			}

			const Hardware::DeviceChassis* chassis = device->getParentChassis();

			if (chassis == dataSourceChassis)
			{
				appDataSource.appendAssociatedSignal(E::LanControllerType::AppData, appSignal.appSignalID());

				m_associatedAppSignals.insert(appSignal.appSignalID());
			}
		}

		return true;
	}
}
