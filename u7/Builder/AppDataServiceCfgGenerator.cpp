#include "AppDataServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/DataSource.h"
#include "../lib/WUtils.h"

class DataSource;

namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(	DbController* db,
															Hardware::SubsystemStorage *subsystems,
															Hardware::Software* software,
															SignalSet* signalSet,
															Hardware::EquipmentSet* equipment,
															BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_subsystems(subsystems)
	{
	}


	AppDataServiceCfgGenerator::~AppDataServiceCfgGenerator()
	{
	}


	bool AppDataServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeAppDataSourcesXml();
		result &= writeAppSignalsXml();
		result &= addLinkToAppSignalsFile();
		result &= writeBatFile();

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

		QByteArray data;
		QXmlStreamWriter xmlWriter(&data);

		XmlWriteHelper xml(xmlWriter);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("AppDataSources");

		m_associatedAppSignals.clear();

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				result = false;
				continue;
			}

			for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
			{
				LmEthernetAdapterNetworkProperties lmNetProperties;

				int adapter = LM_ETHERNET_ADAPTER2;

				if (channel == 1)
				{
					adapter = LM_ETHERNET_ADAPTER3;
				}

				result &= lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, adapter, m_log);

				int lmNumber = 0;
				int lmChannel = 0;
				QString lmSubsystem;
				quint32 lmAppLANDataUID = 0;

				result &= DeviceHelper::getIntProperty(lm, "LMNumber", &lmNumber, m_log);
				result &= DeviceHelper::getIntProperty(lm, "SubsystemChannel", &lmChannel, m_log);
				result &= DeviceHelper::getStrProperty(lm, "SubsystemID", &lmSubsystem, m_log);

				int dataUID = 0;

				result &= DeviceHelper::getIntProperty(lm, "AppLANDataUID", &dataUID, m_log);

				lmAppLANDataUID = dataUID;

				if (result == false)
				{
					break;
				}

				if (lmNetProperties.appDataServiceID == m_software->equipmentIdTemplate())
				{
					int lmSubsystemID = 0;

					int subsystemsCount = m_subsystems->count();

					for(int i = 0; i < subsystemsCount; i++)
					{
						std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);

						if (subsystem->subsystemId() == lmSubsystem)
						{
							lmSubsystemID = subsystem->key();
							break;
						}
					}

					DataSource ds;

					ds.setLmChannel(channel);
					ds.setLmSubsystem(lmSubsystem);
					ds.setLmSubsystemID(lmSubsystemID);
					ds.setLmNumber(lmNumber);
					ds.setLmDataType(DataSource::DataType::App);
					ds.setLmEquipmentID(lm->equipmentIdTemplate());
					ds.setLmModuleType(lm->moduleType());
					ds.setLmCaption(lm->caption());
					ds.setLmDataID(lmAppLANDataUID);
					ds.setLmAdapterID(lmNetProperties.adapterID);
					ds.setLmDataEnable(lmNetProperties.appDataEnable);
					ds.setLmAddressStr(lmNetProperties.appDataIP);
					ds.setLmPort(lmNetProperties.appDataPort);

					result &= findAppDataSourceAssociatedSignals(ds);	// inside fills m_associatedAppSignals also

					ds.writeToXml(xml);
				}
			}

			if (result == false)
			{
				break;
			}
		}

		xml.writeEndElement();	// </AppDataSources>
		xml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, "AppDataSources.xml", CFG_FILE_ID_DATA_SOURCES, "", data);

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

/*		int unitsCount = unitInfo.count();

		for (int i = 0; i < unitsCount; i++)
		{
			xml.writeStartElement("Unit");

			xml.writeIntAttribute("ID", unitInfo.keyAt(i));
			xml.writeStringAttribute("Caption", unitInfo[i]);

			xml.writeEndElement();
		}*/

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

/*			if (!dataFormatInfo.contains(signal.dataFormatInt()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong dataFormat field").arg(signal.appSignalID()));
				hasWrongField = true;
			}*/

/*			if (!unitInfo.contains(signal.unitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong unitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (!unitInfo.contains(signal.inputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputUnitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (!unitInfo.contains(signal.outputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputUnitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (signal.inputSensorType() < 0 || signal.inputSensorType() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (signal.outputSensorType() < 0 || signal.outputSensorType() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}*/

			if (signal.outputMode() < 0 || signal.outputMode() >= OUTPUT_MODE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputRangeMode field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

/*			if (TO_INT(signal.inOutType()) < 0 || TO_INT(signal.inOutType()) >= IN_OUT_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inOutType field").arg(signal.appSignalID()));
				hasWrongField = true;
			}*/

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

		//m_cfgXml->addLinkToFile(buildFile);

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

		content += "AppDataSrv";
		content += " -e";
		content += " -cfgip1=127.0.0.1";
		content += " -id=" + m_software->equipmentIdTemplate() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::BAT_DIR, m_software->equipmentIdTemplate() + ".bat", content);

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
