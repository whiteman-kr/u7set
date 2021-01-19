#include "../lib/Subsystem.h"
#include "TuningServiceCfgGenerator.h"
#include "Context.h"


namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(Context* context,
														 Hardware::Software* software) :
		SoftwareCfgGenerator(context, software),
		m_tuningDataStorage(context->m_tuningDataStorage.get())
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}

	bool TuningServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TuningServiceSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<TuningServiceSettings>(profile, settingsGetter);
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
			if (writeTuningSources() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool TuningServiceCfgGenerator::writeTuningSources()
	{
		std::shared_ptr<const TuningServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<TuningServiceSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		QByteArray fileData;

		bool result = true;

		QVector<Tuning::TuningSource> tuningSources;

		quint32 receivingNetmask = settings->tuningDataNetmask.toIPv4Address();

		quint32 receivingSubnet = settings->tuningDataIP.address32() & receivingNetmask;

		for(Hardware::DeviceModule* lm : m_context->m_lmModules)
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

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, DataSource::DataType::Tuning,
													   lanController.m_place,
													   lanController.m_type,
													   m_context,
													   &ts);
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
									  settings->tuningDataIP.addressStr(),
									  m_software->equipmentIdTemplate());
					result = false;
					continue;
				}

				Tuning::TuningData* tuningData = m_context->m_tuningDataStorage->value(lm->equipmentId(), nullptr);

				if(tuningData != nullptr)
				{
					ts.setTuningData(tuningData);
				}
				else
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString(tr("Tuning data for LM '%1' is not found")).
														arg(lm->equipmentIdTemplate()));
					result = false;
				}

				tuningSources.push_back(ts);
			}
		}

		result &= DataSourcesXML<Tuning::TuningSource>::writeToXml(tuningSources, &fileData);

		RETURN_IF_FALSE(result)

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::TUNING_SOURCES_XML, CfgFileId::TUNING_SOURCES, "", fileData);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
