#include "../lib/ServiceSettings.h"
#include "../u7/Subsystem.h"
#include "TuningServiceCfgGenerator.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(DbController* db,
															Hardware::SubsystemStorage *subsystems,
															Hardware::Software* software,
															SignalSet* signalSet,
															Hardware::EquipmentSet* equipment,
															Tuning::TuningDataStorage *tuningDataStorage,
															const LmsUniqueIdMap& lmsUniqueIdMap,
															BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_tuningDataStorage(tuningDataStorage),
		m_subsystems(subsystems),
		m_lmsUniqueIdMap(lmsUniqueIdMap)
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

		bool result = false;

		do
		{
			if (writeSettings() == false) break;
			if (writeTuningLMs() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

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

		xml.writeStartElement("TuningSources");

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

			if (lmNetProperties.tuningEnable == false)
			{
				continue;
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

			quint64 uniqueID = m_lmsUniqueIdMap.value(lm->equipmentIdTemplate(), 0);

			if (uniqueID == 0)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("UniqueID of LM '%1' is not found")).arg(lm->equipmentIdTemplate()));
				result = false;
			}

			Tuning::TuningSource ds;

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

			ds.setLmUniqueID(uniqueID);

			Tuning::TuningData* tuningData = m_tuningDataStorage->value(lm->equipmentId(), nullptr);

			if(tuningData != nullptr)
			{
				ds.setTuningData(tuningData);
			}
			else
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Tuning data for LM '%1' is not found")).arg(lm->equipmentIdTemplate()));
				result = false;
			}

			ds.writeToXml(xml);
		}

		xml.writeEndElement();				//	</TuningSources>

		return result;
	}


	bool TuningServiceCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "TuningSrv.exe";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool TuningServiceCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./TuningSrv";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
