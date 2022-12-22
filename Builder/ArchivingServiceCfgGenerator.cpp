#include "ArchivingServiceCfgGenerator.h"
#include "../UtilsLib/WUtils.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/SoftwareSettingsGetter.h"

namespace Builder
{

	ArchivingServiceCfgGenerator::ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	ArchivingServiceCfgGenerator::~ArchivingServiceCfgGenerator()
	{
	}

	bool ArchivingServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		ArchivingServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<ArchivingServiceSettingsGetter>(profile, settingsGetter);

		result &=  writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &=  writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool ArchivingServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeArchSignalsFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool ArchivingServiceCfgGenerator::writeArchSignalsFile()
	{
		TEST_PTR_RETURN_FALSE(m_signalSet);
		TEST_PTR_RETURN_FALSE(m_software);

		Proto::ArchSignals msg;

		int count = static_cast<int>(m_signalSet->count());

		for(int i = 0; i < count; i++)
		{
			AppSignal& s = (*m_signalSet)[i];

			if (s.isAnalog() == true || s.isDiscrete() == true)
			{
				Proto::ArchSignal* archSignal = msg.add_archsignals();

				TEST_PTR_CONTINUE(archSignal);

				archSignal->set_hash(calcHash(s.appSignalID()));
				archSignal->set_isanalog(s.isAnalog());
				archSignal->set_appsignalid(s.appSignalID().toStdString());
			}
		}

		int size = static_cast<int>(msg.ByteSizeLong());

		char* ptr = new char[size];

		msg.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(ptr));

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ArchSignals.proto", QByteArray::fromRawData(ptr, size), true);

		delete [] ptr;

		TEST_PTR_RETURN_FALSE(buildFile);

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool ArchivingServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														  const ArchivingServiceSettings& settings,
														  E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgServiceIP1, settings.cfgServiceIP2, os, true);

		if (cmdLine.isEmpty() == true)
		{
			return false;
		}

		cmdLine += QString(" -location=%1").arg(settings.archiveLocation);

		content += cmdLine;

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);
		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
