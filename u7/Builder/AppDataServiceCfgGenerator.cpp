#include "AppDataServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/DataSource.h"

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
		result &= addLinkToAppSignalsFile();

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
