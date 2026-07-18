#include "ArchivingServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "AppDataServiceCfgGenerator.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/SoftwareSettings.h"

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

	bool ArchivingServiceCfgGenerator::generateConfigurationStep2()
	{ 
		return writeArchInfoV3File();
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

			Proto::ArchSignal ps;

			bool res = copyArchSignal(s, &ps);

			if (res == false)
			{
				continue;
			}

			Proto::ArchSignal* archSignal = archInfo.add_archsignal();
			TEST_PTR_BREAK(archSignal);

			*archSignal = ps;
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

	bool ArchivingServiceCfgGenerator::writeArchInfoV3File()
	{ 
		bool result = true;

		Proto::ArchInfoV3 archInfo;

		m_buildResultWriter->buildInfo().saveToProto(archInfo.mutable_buildinfo());
		archInfo.set_archiveserviceid(m_software->equipmentIdTemplate().toStdString());

		//qDebug() << "Archiving service " << m_software->equipmentIdTemplate();

		for (const auto& [swID, sw] : m_context->m_software)
		{
			if (sw->softwareType() != E::SoftwareType::AppDataService)
			{
				continue;
			}

			QString archServiceID;
			 
			bool res = DeviceHelper::getStrProperty(sw, EquipmentPropNames::ARCH_SERVICE_ID, &archServiceID, m_log);

			if (res == false)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			if (archServiceID != m_software->equipmentIdTemplate())
			{
				continue;
			}

			QString appDatSrvID = sw->equipmentIdTemplate();

			auto it = m_context->m_swCfgGens.find(appDatSrvID);

			if (it == m_context->m_swCfgGens.end())
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			std::shared_ptr<AppDataServiceCfgGenerator> appDataServiceCfgGen = std::dynamic_pointer_cast<AppDataServiceCfgGenerator>(it->second);

			const std::set<Hash>& acquiredSignals = appDataServiceCfgGen->acquiredAppSignals();

			if (appDataServiceCfgGen == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			Proto::ClientArchSignals* clientArchSignals = archInfo.add_clientarchsignals();

			clientArchSignals->set_clientid(appDatSrvID.toStdString());

			// qDebug() << "Client " << appDatSrvID;

			for (Hash h : acquiredSignals)
			{
				const AppSignal* s = m_signalSet->getSignalByHash(h);

				if (s == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				Proto::ArchSignal ps;

				bool res = copyArchSignal(s, &ps);

				if (res == false)
				{
					continue;
				}

				// qDebug() << "Signal " << s->appSignalID();

				Proto::ArchSignal* archSignal = clientArchSignals->add_archsignal();
				TEST_PTR_BREAK(archSignal);

				*archSignal = ps;
			}
		}

		RETURN_IF_FALSE(result);

		qsizetype size = static_cast<qsizetype>(archInfo.ByteSizeLong());

		QByteArray data;
		data.resize(size);

		archInfo.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(data.data()));

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
															File::ARCH_INFO_V3_PROTO,
															data,
															true);
		TEST_PTR_RETURN_FALSE(buildFile);

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool ArchivingServiceCfgGenerator::copyArchSignal(const AppSignal* s, Proto::ArchSignal* ps) const
	{
		TEST_PTR_RETURN_FALSE(s);
		TEST_PTR_RETURN_FALSE(ps);

		switch (s->signalType())
		{ 
		case E::SignalType::Discrete:
		case E::SignalType::Analog:
			break;

		default:
			return false;
		}	

		ps->set_appsignalid(s->appSignalID().toStdString());
		ps->set_signaltype(TO_INT(s->signalType()));

		if (s->signalType() == E::SignalType::Analog)
		{
			ps->set_lowlimit(s->lowEngineeringUnits());
			ps->set_highlimit(s->highEngineeringUnits());
			ps->set_unit(s->unit().toStdString());
			ps->set_fineaperture(s->fineAperture());
			ps->set_coarseaperture(s->coarseAperture());
		}

		return true;
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
