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
		bool result = m_settings.readFromDevice(m_context, m_software);

		RETURN_IF_FALSE(result);

		if (m_context->m_projectProperties.safetyProject() == true && m_settings.singleLmControl == false)
		{
			// TuningService (%1) cannot be used for multi LM control in Safety Project. Turn On option %1.SingleLmControl or override behaviour in menu Project->Project Properties...->Safety Project.
			//
			m_log->errEQP6201(equipmentID());
			return false;
		}

		return getSettingsXml(m_cfgXml->xmlWriter());
	}

	bool TuningServiceCfgGenerator::writeTuningSources()
	{
		QByteArray fileData;

		bool result = true;

		result &= DataSourcesXML<Tuning::TuningSource>::writeToXml(m_settings.tuningSources, &fileData);

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
