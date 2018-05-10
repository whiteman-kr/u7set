#include "../lib/ServiceSettings.h"
#include "../u7/Subsystem.h"
#include "../TuningService/TuningSource.h"
#include "TuningServiceCfgGenerator.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(DbController* db,
															const Hardware::SubsystemStorage* subsystems,
															Hardware::Software* software,
															SignalSet* signalSet,
															Hardware::EquipmentSet* equipment,
															Tuning::TuningDataStorage *tuningDataStorage,
															const LmsUniqueIdMap& lmsUniqueIdMap,
															BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_tuningDataStorage(tuningDataStorage),
		m_lmsUniqueIdMap(lmsUniqueIdMap)
	{
		initSubsystemKeyMap(&m_subsystemKeyMap, subsystems);
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

		bool result = false;

		do
		{
			if (writeSettings() == false) break;
			if (writeTuningSources() == false) break;
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


	bool TuningServiceCfgGenerator::writeTuningSources()
	{
		bool result = true;

		QVector<Tuning::TuningSource> tuningSources;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			Tuning::TuningSource ts;

			result &= ts.getLmPropertiesFromDevice(lm, DataSource::DataType::Tuning,
												   DataSource::LM_ETHERNET_ADAPTER1,
												   m_subsystemKeyMap,
												   m_lmsUniqueIdMap,
												   m_log);
			if (result == false)
			{
				break;
			}

			if (ts.lmDataEnable() == false || ts.serviceID() != m_software->equipmentIdTemplate())
			{
				continue;
			}

			Tuning::TuningData* tuningData = m_tuningDataStorage->value(lm->equipmentId(), nullptr);

			if(tuningData != nullptr)
			{
				ts.setTuningData(tuningData);
			}
			else
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Tuning data for LM '%1' is not found")).arg(lm->equipmentIdTemplate()));
				result = false;
			}

			tuningSources.append(ts);
		}

		if (result == false)
		{
			return false;
		}

		QByteArray fileData;
		result &= DataSourcesXML<Tuning::TuningSource>::writeToXml(tuningSources, &fileData);

		if (result == false)
		{
			return false;
		}

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, FILE_TUNING_SOURCES_XML, CFG_FILE_ID_TUNING_SOURCES, "", fileData);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
