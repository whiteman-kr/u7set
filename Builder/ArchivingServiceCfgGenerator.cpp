#include "ArchivingServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../Proto/ArchSignal.pb.h"

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
		TEST_PTR_RETURN_FALSE(m_buildResultWriter);

		Proto::ArchInfo archInfo;

		m_buildResultWriter->buildInfo().saveToProto(archInfo.mutable_buildinfo());

		archInfo.set_archiveserviceid(m_software->equipmentIdTemplate().toStdString());

		for(const AppSignal* s : *m_signalSet)
		{
			if (s->acquire() == false)
			{
				continue;
			}

			switch(s->signalType())
			{
			case E::SignalType::Discrete:
				{
					Proto::ArchSignal* archSignal = archInfo.add_archsignal();

					TEST_PTR_BREAK(archSignal);

					archSignal->set_appsignalid(s->appSignalID().toStdString());
					archSignal->set_signaltype(TO_INT(s->signalType()));
				}
				break;

			case E::SignalType::Analog:
				{
					Proto::ArchSignal* archSignal = archInfo.add_archsignal();

					TEST_PTR_BREAK(archSignal);

					archSignal->set_appsignalid(s->appSignalID().toStdString());
					archSignal->set_signaltype(TO_INT(s->signalType()));
					archSignal->set_lowlimit(s->lowEngineeringUnits());
					archSignal->set_highlimit(s->highEngineeringUnits());
					archSignal->set_unit(s->unit().toStdString());
					archSignal->set_fineaperture(s->fineAperture());
					archSignal->set_coarseaperture(s->coarseAperture());
				}
				break;

			case E::SignalType::Bus:
				break;

			default:
				Q_ASSERT(false);
			}
		}

		int size = static_cast<int>(archInfo.ByteSizeLong());

		char* ptr = new char[size];

		archInfo.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(ptr));

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
															File::ARCH_INFO_PROTO,
															QByteArray::fromRawData(ptr, size), true);
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
