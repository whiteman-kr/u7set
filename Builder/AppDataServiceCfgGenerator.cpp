#include "AppDataServiceCfgGenerator.h"
#include "Builder.h"
#include "../lib/SignalProperties.h"

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

		return m_settingsSet.addProfile<AppDataServiceSettings>(profile, settingsGetter);
	}

	bool AppDataServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeAppDataSourcesXml() == false) break;
			if (writeAppSignalsXml() == false) break;
			if (addLinkToAppSignalsFile() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

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

			std::shared_ptr<LmDescription> lmDescription = m_context->m_lmDescriptions->get(lm);

			if (lmDescription == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("LmDescription is not found for module %1").arg(lm->equipmentIdTemplate()));
				result = false;
				continue;
			}

			const LmDescription::Lan& lan = lmDescription->lan();

			int connectedAdaptersCount = 0;

			for(const LmDescription::LanController& lanController : lan.m_lanControllers)
			{
				if (lanController.isProvideAppData() == false)
				{
					continue;
				}

				DataSource ds;

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, DataSource::DataType::App,
																			lanController.m_place,
																			lanController.m_type,
																			m_context,
																			&ds);

				if (ds.lmDataEnable() == false || ds.serviceID() != m_software->equipmentIdTemplate())
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

				if ((ds.lmAddress().toIPv4Address() & receivingNetmask) != receivingSubnet)
				{
					// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
					//
					m_log->errCFG3043(ds.lmAddress().toString(),
									  ds.lmAdapterID(),
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

		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("AppSignals");
		xml.writeIntAttribute("buildID", m_buildResultWriter->buildInfo().id);

		// Writing units
		xml.writeStartElement("Units");
		xml.writeIntAttribute("Count", 0);
		xml.writeEndElement();				// Units

		QVector<Signal*> signalsToWrite;

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			Signal& signal = (*m_signalSet)[i];

			if (m_associatedAppSignals.contains(signal.appSignalID()) == false)
			{
				continue;
			}

			bool hasWrongField = false;

			if (signal.isSpecPropExists(SignalProperties::outputModeCaption) == true && E::contains<E::OutputMode>(signal.outputMode()) == false)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputRangeMode field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			switch (static_cast<E::ByteOrder>(signal.byteOrderInt()))
			{
				case E::ByteOrder::LittleEndian:
				case E::ByteOrder::BigEndian:
					break;
				default:
					LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong byteOrder field").arg(signal.appSignalID()));
					hasWrongField = true;
			}

			if (hasWrongField)
			{
				continue;
			}

			signalsToWrite.append(&signal);
		}

		// Writing signals
		//
		xml.writeStartElement("Signals");
		xml.writeIntAttribute("Count", signalsToWrite.count());

		for(Signal* signal : signalsToWrite)
		{
			signal->writeToXml(xml);
		}

		signalsToWrite.clear();

		xml.writeEndElement();	// </Signals>
		xml.writeEndElement();	// </AppSignals>
		xml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), "AppSignals.xml", CfgFileId::APP_SIGNALS, "",  data);

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

	bool AppDataServiceCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "AppDataSrv.exe";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}
		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool AppDataServiceCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./AppDataSrv";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool AppDataServiceCfgGenerator::findAppDataSourceAssociatedSignals(DataSource& appDataSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(appDataSource.lmEquipmentID()).get();

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const Hardware::DeviceChassis* dataSourceChassis = lm->getParentChassis();

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			const Signal& appSignal =  (*m_signalSet)[i];

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
				appDataSource.addAssociatedSignal(appSignal.appSignalID());

				m_associatedAppSignals.insert(appSignal.appSignalID(), true);
			}
		}

		return true;
	}
}
