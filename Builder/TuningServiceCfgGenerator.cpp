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
