#include "AppDataServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/WUtils.h"
#include "Builder.h"

class DataSource;

namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(	DbController* db,
															const Hardware::SubsystemStorage* subsystems,
															Hardware::Software* software,
															SignalSet* signalSet,
															Hardware::EquipmentSet* equipment,
															const QHash<QString, quint64>& lmUniqueIdMap,
															BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_lmUniqueIdMap(lmUniqueIdMap)
	{
		initSubsystemKeyMap(&m_subsystemKeyMap, subsystems);
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
		AppDataServiceSettings dasSettings;

		bool result = true;

		result = dasSettings.readFromDevice(m_equipment, m_software, m_log);

		if (result == false)
		{
			return false;
		}

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result = dasSettings.writeToXml(xml);

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppDataSourcesXml()
	{
		bool result = true;

		m_associatedAppSignals.clear();

		QVector<DataSource> dataSources;

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
					m_log->errCFG3028(lm->equipmentIdTemplate(), m_software->equipmentIdTemplate());
					result = false;
					break;
				}

				connectedAdaptersCount++;

				result &= findAppDataSourceAssociatedSignals(ds);	// inside fills m_associatedAppSignals also

				dataSources.append(ds);
			}
		}

		if (result == false)
		{
			return false;
		}

		//

		QByteArray fileData;
		result &= DataSourcesXML<DataSource>::writeToXml(dataSources, &fileData);

		if (result == false)
		{
			return false;
		}

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

			if (signal.outputMode() < 0 || signal.outputMode() >= OUTPUT_MODE_COUNT)
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
		BuildFile* buildFile = m_buildResultWriter->getBuildFileByID(CFG_FILE_ID_APP_SIGNAL_SET);

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
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, QString("Not found LM with ID '%1'").arg(appDataSource.lmEquipmentID()));
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
