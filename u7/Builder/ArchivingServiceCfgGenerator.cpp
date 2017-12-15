#include "ArchivingServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/WUtils.h"

namespace Builder
{

	ArchivingServiceCfgGenerator::ArchivingServiceCfgGenerator(DbController* db,
															   Hardware::Software* software,
															   SignalSet* signalSet,
															   Hardware::EquipmentSet* equipment,
															   BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	ArchivingServiceCfgGenerator::~ArchivingServiceCfgGenerator()
	{
	}


	bool ArchivingServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeBatFile();

		return result;
	}


	bool ArchivingServiceCfgGenerator::writeSettings()
	{
		ArchivingServiceSettings settings;

		bool result = true;

		result &= settings.readFromDevice(m_software, m_log);

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result &= settings.writeToXml(xml);

		result &= writeArchSignalsFile();

		return result;
	}

	bool ArchivingServiceCfgGenerator::writeArchSignalsFile()
	{
		TEST_PTR_RETURN_FALSE(m_signalSet);
		TEST_PTR_RETURN_FALSE(m_software);

		Proto::ArchSignals msg;

		int count = m_signalSet->count();

		for(int i = 0; i < count; i++)
		{
			Signal& s = (*m_signalSet)[i];

			if (s.isAnalog() == true || s.isDiscrete() == true)
			{
				Proto::ArchSignal* archSignal = msg.add_archsignals();

				TEST_PTR_CONTINUE(archSignal);

				archSignal->set_hash(calcHash(s.appSignalID()));
				archSignal->set_isanalog(s.isAnalog());
				archSignal->set_appsignalid(s.appSignalID().toStdString());
			}
		}

		int size = msg.ByteSize();

		char* ptr = new char[size];

		msg.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(ptr));

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ArchSignals.proto", QByteArray::fromRawData(ptr, size), true);

		delete [] ptr;

		TEST_PTR_RETURN_FALSE(buildFile);

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool ArchivingServiceCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "ArchSrv.exe";
		content += " -e";

		QString cfgIP1;
		QString cfgIP2;
		if (getConfigIp(cfgIP1, cfgIP2) == false)
		{
			return false;
		}

		if (cfgIP1.isEmpty() == false)
		{
			content += " -cfgip1=" + cfgIP1;
		}

		if (cfgIP2.isEmpty() == false && cfgIP1 != cfgIP2)
		{
			content += " -cfgip2=" + cfgIP1;
		}

		content += " -id=" + m_software->equipmentIdTemplate() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::BAT_DIR, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

}
