#include "ArchivingServiceCfgGenerator.h"
#include "../lib/WUtils.h"

namespace Builder
{

	ArchivingServiceCfgGenerator::ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}


	ArchivingServiceCfgGenerator::~ArchivingServiceCfgGenerator()
	{
	}


	bool ArchivingServiceCfgGenerator::generateConfiguration()
	{
		bool result = false;

		do
		{
			if (writeSettings() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}


	bool ArchivingServiceCfgGenerator::writeSettings()
	{
		bool result = true;

		result &= m_settings.readFromDevice(m_software, m_log);

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result &= m_settings.writeToXml(xml);

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

		QString parameters;
		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		parameters += QString(" -location=%1").arg(m_settings.archiveLocation);

		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(Builder::DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool ArchivingServiceCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./ArchSrv";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		parameters += QString(" -location=%1").arg(m_settings.archiveLocation);

		content += parameters;

		BuildFile* buildFile = m_buildResultWriter->addFile(Builder::DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

}
