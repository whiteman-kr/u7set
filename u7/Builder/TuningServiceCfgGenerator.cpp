#include "../include/ServiceSettings.h"
#include "TuningServiceCfgGenerator.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(DbController* db,
														 Hardware::Software* software,
														 SignalSet* signalSet,
														 Hardware::EquipmentSet* equipment,
														 TuningDataStorage *tuningDataStorage,
														 BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_tuningDataStorage(tuningDataStorage)
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}


	bool TuningServiceCfgGenerator::generateConfiguration()
	{
		if (m_tuningDataStorage == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		result &= writeSettings();

//		result &= findTuningSignals();

		result &= writeTuningLMs();
//		result &= writeTuningSignals();

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

			TuningDataSource ds;

			ds.setChannel(0);
			ds.setDataType(DataSource::DataType::Tuning);
			ds.setLmEquipmentID(lm->equipmentId());
			ds.setLmCaption(lm->caption());
			ds.setLmAdapterID(lmNetProperties.adapterID);
			ds.setLmDataEnable(lmNetProperties.tuningEnable);
			ds.setLmAddressStr(lmNetProperties.tuningIP);
			ds.setLmPort(lmNetProperties.tuningPort);

			if (m_tuningDataStorage->contains(lm->equipmentId()) == true)
			{
				TuningData* tuningData = (*m_tuningDataStorage)[lm->equipmentId()];

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
