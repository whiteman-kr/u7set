#include "../include/ServiceSettings.h"
#include "TuningServiceCfgGenerator.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(DbController* db,
														 Hardware::Software* software,
														 SignalSet* signalSet,
														 Hardware::EquipmentSet* equipment,
														 BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}


	bool TuningServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeTuningLMs();
		result &= writeTuningSignals();

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
				DataSource ds;

				ds.setChannel(0);
				ds.setDataType(DataSource::DataType::Tuning);
				ds.setLmStrID(lm->equipmentIdTemplate());
				ds.setLmCaption(lm->caption());
				ds.setLmAdapterStrID(lmNetProperties.adapterID);
				ds.setLmDataEnable(lmNetProperties.tuningEnable);
				ds.setLmAddressStr(lmNetProperties.tuningIP);
				ds.setLmPort(lmNetProperties.tuningPort);

				result &= findAssociatedTuningSignals(ds);

				ds.writeToXml(xml);
			}
		}

		xml.writeEndElement();				//	</TuningLMs>

		return result;
	}


	bool TuningServiceCfgGenerator::findAssociatedTuningSignals(DataSource& tuningSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(tuningSource.lmStrID());

		if (lm == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Not found LM with ID '%1'").arg(tuningSource.lmStrID()));
			return false;
		}

		const Hardware::DeviceChassis* tuningSourceChassis = lm->getParentChassis();

		for(Signal* signal : m_tuningSignals)
		{
			QString signalEquipmentID = signal->equipmentID();

			if (signalEquipmentID.isEmpty())
			{
				continue;
			}

			Hardware::DeviceObject* device = m_equipment->deviceObject(signalEquipmentID);

			if (device == nullptr)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal '%1' bound with an unknown device '%2'").
					arg(signal->appSignalID()).arg(signalEquipmentID));
				continue;
			}

			const Hardware::DeviceChassis* chassis = device->getParentChassis();

			if (chassis == tuningSourceChassis)
			{
				tuningSource.addAssociatedSignal(signal->appSignalID());
			}
		}

		return true;
	}


	bool TuningServiceCfgGenerator::findTuningSignals()
	{
		m_tuningSignals.clear();

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			Signal& appSignal =  (*m_signalSet)[i];

			if (appSignal.enableTuning() == false)
			{
				continue;
			}

			m_tuningSignals.append(&appSignal);
		}

		return true;
	}


	bool TuningServiceCfgGenerator::writeTuningSignals()
	{
		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		/*xml.setAutoFormatting(true);
		xml.writeStartDocument();*/

		xml.writeStartElement("TuningSignals");
		xml.writeIntAttribute("Count", m_tuningSignals.count());

		for(Signal* signal : m_tuningSignals)
		{
			signal->writeToXml(xml);
		}

		xml.writeEndElement();			//	</TuningSignals>

		/*xml.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "TuningSignals.xml", CFG_FILE_ID_TUNING_SIGNALS, "", xmlData);
		m_cfgXml->addLinkToFile(m_subDir, "appSignals.xml");*/

		return true;
	}

}
