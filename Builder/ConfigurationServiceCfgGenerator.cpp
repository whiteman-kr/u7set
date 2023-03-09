#include "ConfigurationServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{

	ConfigurationServiceCfgGenerator::ConfigurationServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	ConfigurationServiceCfgGenerator::~ConfigurationServiceCfgGenerator()
	{
	}

	bool ConfigurationServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		CfgServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<CfgServiceSettingsGetter>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool ConfigurationServiceCfgGenerator::generateConfigurationStep1()
	{
		return true;
	}

	bool ConfigurationServiceCfgGenerator::writeRunScriptFile(const QString& profile,
															  const CfgServiceSettings& settings,
															  E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		content += getCommentStart(os) + " To run simulation append param -mode=simulation to command line\n\n";

		content += getCommandLine(profile, settings.clientRequestIP, os);

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);
		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	QString ConfigurationServiceCfgGenerator::getCommandLine(const QString& profile,
															 const HostAddressPort& clientRequestIP,
															 E::OS os) const
	{
		QString cmdLine;

		switch(os)
		{
		case E::OS::Windows:
			cmdLine = "CfgSrv.exe";
			break;

		case E::OS::Linux:
			cmdLine = "./CfgSrv";
			break;

		default:
			Q_ASSERT(false);
			return QString();
		}

		cmdLine += " -e";
		cmdLine += " -id=" + m_software->equipmentIdTemplate();
		cmdLine += " -profile=" + profile;

		// build path
		//
		QString appDataPath = QDir::fromNativeSeparators(m_buildResultWriter->outputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1/build")
				.arg(m_dbController->currentProject().projectName());

		cmdLine += " -b=" + appDataPath + "/" + buildDir;

		cmdLine += " -ip=" + clientRequestIP.addressPortStr() + "\n";

		return cmdLine;
	}
}
