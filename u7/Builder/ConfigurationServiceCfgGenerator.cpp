#include "ConfigurationServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/WUtils.h"
#include "../Settings.h"

namespace Builder
{

	ConfigurationServiceCfgGenerator::ConfigurationServiceCfgGenerator(DbController* db,
															   Hardware::Software* software,
															   SignalSet* signalSet,
															   Hardware::EquipmentSet* equipment,
															   BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	ConfigurationServiceCfgGenerator::~ConfigurationServiceCfgGenerator()
	{
	}


	bool ConfigurationServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeBatFile();

		return result;
	}


	bool ConfigurationServiceCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "CfgSrv";
		content += " -e";
		content += " -id=" + m_software->equipmentIdTemplate();

		// build path
		//
		QString appDataPath = QDir::fromNativeSeparators(theSettings.buildOutputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1-%2/build")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildResultWriter->buildInfo().typeStr());

		content += " -b=" + appDataPath + "/" + buildDir;

		content += " -ip=" + m_software->propertyByCaption("ClientRequestIP")->value().toString() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::BAT_DIR, m_software->equipmentIdTemplate() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

}
