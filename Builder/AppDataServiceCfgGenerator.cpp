#include "AppDataServiceCfgGenerator.h"
#include "Builder.h"

class DataSource;

namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(Context* context,
														   Hardware::Software* software,
														   const QHash<QString, quint64>& lmUniqueIdMap) :
		SoftwareCfgGenerator(context, software),
		m_lmUniqueIdMap(lmUniqueIdMap)
	{
		assert(context);

		initSubsystemKeyMap(&m_subsystemKeyMap, context->m_subsystems.get());
	}

	AppDataServiceCfgGenerator::~AppDataServiceCfgGenerator()
	{
	}

	bool AppDataServiceCfgGenerator::generateConfiguration()
	{
		bool result = false;

		do
		{
			if (writeSettings() == false) break;
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

	bool AppDataServiceCfgGenerator::writeSettings()
	{
		bool result = m_settings.readFromDevice(m_equipment, m_software, m_log);

		RETURN_IF_FALSE(result);

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result = m_settings.writeToXml(xml);

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppDataSourcesXml()
	{
		bool result = true;

		m_associatedAppSignals.clear();

		QVector<DataSource> dataSources;

		quint32 receivingNetmask = m_settings.appDataReceivingNetmask.toIPv4Address();

		quint32 receivingSubnet = m_settings.appDataReceivingIP.address32() & receivingNetmask;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			int connectedAdaptersCount = 0;

			for(int adapter = DataSource::LM_ETHERNET_ADAPTER2; adapter <= DataSource::LM_ETHERNET_ADAPTER3; adapter++)
			{
				DataSource ds;

				result &= ds.getLmPropertiesFromDevice(lm, DataSource::DataType::App, adapter, m_subsystemKeyMap, m_lmUniqueIdMap, m_log);

				if (ds.lmDataEnable() == false || ds.serviceID() != m_software->equipmentIdTemplate())
				{
					continue;
				}

				if (connectedAdaptersCount > 0)
				{
					// Etherent adapters 2 and 3 of LM %1 are connected to same AppDataService %2.
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
									  m_settings.appDataReceivingIP.addressStr(),
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

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, FILE_APP_DATA_SOURCES_XML, CFG_FILE_ID_APP_DATA_SOURCES, "", fileData);

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

			if (E::contains<E::OutputMode>(signal.outputMode()) == false)
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

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, "AppSignals.xml", CFG_FILE_ID_APP_SIGNALS, "",  data);

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

		BuildFile* buildFile = m_buildResultWriter->getBuildFileByID(DIR_COMMON, CFG_FILE_ID_APP_SIGNAL_SET);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool AppDataServiceCfgGenerator::findAppDataSourceAssociatedSignals(DataSource& appDataSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(appDataSource.lmEquipmentID());

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

			Hardware::DeviceObject* device = m_equipment->deviceObject(appSignalEquipmentID);

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
