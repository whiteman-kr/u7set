#include "../include/ServiceSettings.h"
#include "../u7/Subsystem.h"
#include "TuningServiceCfgGenerator.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(	DbController* db,
															Hardware::SubsystemStorage *subsystems,
															Hardware::Software* software,
															SignalSet* signalSet,
															Hardware::EquipmentSet* equipment,
															Tuning::TuningDataStorage *tuningDataStorage,
															BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_tuningDataStorage(tuningDataStorage),
		m_subsystems(subsystems)
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}


	bool TuningServiceCfgGenerator::generateConfiguration()
	{
		if (m_tuningDataStorage == nullptr ||
			m_subsystems == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		result &= writeSettings();
		result &= writeTuningLMs();

		return result;
	}


	bool TuningServiceCfgGenerator::writeSettings()
	{
		TuningServiceSettings settings;

		bool result = true;

		result &= settings.readFromDevice(m_software, m_log);

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result &= settings.writeToXml(xml);

		return result;
	}


	bool TuningServiceCfgGenerator::writeTuningLMs()
	{
		m_tuningLMs.clear();

		bool result = true;

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		xml.writeStartElement("TuningLMs");

		QList<Hardware::DeviceModule*> tuningLMs;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				result = false;
				continue;
			}

			LmEthernetAdapterNetworkProperties lmNetProperties;

			result &= lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, LM_ETHERNET_ADAPTER1, m_log);

			if (result == false)
			{
				break;
			}

			if (lmNetProperties.tuningServiceID == m_software->equipmentIdTemplate())
			{
				tuningLMs.append(lm);
			}
		}

		xml.writeIntAttribute("Count", tuningLMs.count());

		for(Hardware::DeviceModule* lm : tuningLMs)
		{
			LmEthernetAdapterNetworkProperties lmNetProperties;

			result &= lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, LM_ETHERNET_ADAPTER1, m_log);

			int lmNumber = 0;
			int lmChannel = 0;
			QString lmSubsystem;

			result &= DeviceHelper::getIntProperty(lm, "LMNumber", &lmNumber, m_log);
			result &= DeviceHelper::getIntProperty(lm, "SubsystemChannel", &lmChannel, m_log);
			result &= DeviceHelper::getStrProperty(lm, "SubsystemID", &lmSubsystem, m_log);

			if (result == false)
			{
				continue;
			}

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

			Tuning::TuningDataSource ds;

			ds.setLmChannel(lmChannel);
			ds.setLmDataType(DataSource::DataType::Tuning);
			ds.setLmEquipmentID(lm->equipmentId());
			ds.setLmNumber(lmNumber);
			ds.setLmModuleType(lm->moduleType());
			ds.setLmSubsystemID(lmSubsystemID);
			ds.setLmSubsystem(lmSubsystem);

			ds.setLmCaption(lm->caption());
			ds.setLmAdapterID(lmNetProperties.adapterID);
			ds.setLmDataEnable(lmNetProperties.tuningEnable);
			ds.setLmAddressStr(lmNetProperties.tuningIP);
			ds.setLmPort(lmNetProperties.tuningPort);

			ds.setLmDataID(0);			// !!! ???

			if (m_tuningDataStorage->contains(lm->equipmentId()) == true)
			{
				Tuning::TuningData* tuningData = (*m_tuningDataStorage)[lm->equipmentId()];

				if(tuningData != nullptr)
				{
					ds.setTuningData(tuningData);
				}
			}

			ds.writeToXml(xml);
		}

		xml.writeEndElement();				//	</TuningLMs>

		return result;
	}
}
