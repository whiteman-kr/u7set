#include "../lib/Subsystem.h"
#include "TuningServiceCfgGenerator.h"
#include "Context.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(Context* context,
														 Hardware::Software* software,
														 const LmsUniqueIdMap& lmsUniqueIdMap) :
		SoftwareCfgGenerator(context, software),
		m_lmsUniqueIdMap(lmsUniqueIdMap),
		m_tuningDataStorage(context->m_tuningDataStorage.get())
	{
		initSubsystemKeyMap(&m_subsystemKeyMap, context->m_subsystems.get());
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

	bool TuningServiceCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_context, m_log);

		if (m_settings.isInitialized() == false)
		{
			bool result = m_settings.readFromDevice(m_software, m_log);

			RETURN_IF_FALSE(result);

			if (m_context->m_projectProperties.safetyProject() == true && m_settings.singleLmControl == false)
			{
				// TuningService (%1) cannot be used for multi LM control in Safety Project. Turn On option %1.SingleLmControl or override behaviour in menu Project->Project Properties...->Safety Project.
				//
				m_log->errEQP6201(equipmentID());
				return false;
			}

			m_settings.setInitialized();
		}

		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);
	}

	bool TuningServiceCfgGenerator::writeSettings()
	{
		return getSettingsXml(m_cfgXml->xmlWriter());
	}

	bool TuningServiceCfgGenerator::writeTuningSources()
	{
		bool result = true;

		QVector<Tuning::TuningSource> tuningSources;

		quint32 receivingNetmask = m_settings.tuningDataNetmask.toIPv4Address();

		quint32 receivingSubnet = m_settings.tuningDataIP.address32() & receivingNetmask;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
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

			for(const LmDescription::LanController& lanController : lan.m_lanControllers)
			{
				if (lanController.isProvideTuning() == false)
				{
					continue;
				}

				Tuning::TuningSource ts;

				result &= ts.getLmPropertiesFromDevice(lm, DataSource::DataType::Tuning,
													   lanController.m_place,
													   lanController.m_type,
				                                       *m_equipment,
													   m_subsystemKeyMap,
													   m_lmsUniqueIdMap,
													   m_log);
				if (result == false)
				{
					continue;
				}

				if (ts.lmDataEnable() == false || ts.serviceID() != m_software->equipmentIdTemplate())
				{
					continue;
				}

				if ((ts.lmAddress().toIPv4Address() & receivingNetmask) != receivingSubnet)
				{
					// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
					//
					m_log->errCFG3043(ts.lmAddress().toString(),
									  ts.lmAdapterID(),
									  m_settings.tuningDataIP.addressStr(),
									  equipmentID());
					result = false;
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
		}

		RETURN_IF_FALSE(result)

		QByteArray fileData;
		result &= DataSourcesXML<Tuning::TuningSource>::writeToXml(tuningSources, &fileData);

		RETURN_IF_FALSE(result)

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
