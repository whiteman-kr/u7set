#include "GatewayServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../GatewayService/GatewayDescriptionParser.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"

namespace Builder
{

	GatewayServiceCfgGenerator::GatewayServiceCfgGenerator(Context* context, Hardware::Software* software)	:
		SoftwareCfgGenerator(context, software)
	{
	}

	bool GatewayServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		GatewayServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = true;

		result &= m_settingsSet.addProfile<GatewayServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool GatewayServiceCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
			m_software->softwareType() != E::SoftwareType::GatewayService ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(m_software);
			assert(m_software->softwareType() == E::SoftwareType::GatewayService);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			return false;
		}

		IssueLogger* log = m_buildResultWriter->log();

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		std::shared_ptr<const GatewayServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<GatewayServiceSettings>();

		LOG_MESSAGE(log, QString("Parsing of %1 gateway description started...").arg(equipmentID()));

		Gateway::GatewaysShared gateways = std::make_shared<Gateway::Gateways>();

		Gateway::Parser parser(m_context->m_signalSet->appSignalSet(), gateways);

		result = parser.parse(settings->gatewayDescription);

		const Gateway::ParserLog& parserLog = parser.log();

		for(const auto& r : parserLog)
		{
			switch(r.msgType)
			{
			case Gateway::LogMsgType::Message:
				{
					QString msg = r.msg;
					msg = msg.mid(0, 1).toUpper() + msg.mid(1);
					LOG_MESSAGE(log, msg);
				}
				break;

			case Gateway::LogMsgType::Warning:
				// Gateway description parsing warning: %1
				//
				log->wrnCFG3052(r.msg);
				break;

			case Gateway::LogMsgType::Error:
				// Gateway description parsing error: %1
				//
				log->errCFG3051(r.msg);
				break;

			default:
				Q_ASSERT(false);
			}
		}

		int errCount = parserLog.errorCount();
		int wrnCount = parserLog.warningCount();

		BuildFile* buildFile = nullptr;

		if (errCount == 0)
		{
			for(const Gateway::GatewayShared gw : *gateways)
			{
				TEST_PTR_CONTINUE(gw);

				const auto& files = gw->files();

				for(const Gateway::File& file : files)
				{
					buildFile = m_buildResultWriter->addFile(
											softwareCfgSubdir() + Separator::DIR + file.gatewayID(),
											file.fileName(), file.fileData());

					if (buildFile == nullptr)
					{
						errCount++;
						result = false;
					}
				}
			}

			QString xmlStr;
			XmlWriteHelper xml(&xmlStr);

			gateways->writeToXml(xml);

			buildFile = m_buildResultWriter->addFile(
									softwareCfgSubdir(),
									File::GATEWAY_DESCRIPTION_XML,
									CfgFileId::GATEWAY_DESCRIPTION,
									QString(),
									xmlStr);

			if (buildFile == nullptr)
			{
				errCount++;
				result = false;
			}
			else
			{
				m_cfgXml->addLinkToFile(buildFile);
			}
		}

		buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
												File::GATEWAY_DESCRIPTION_TXT,
												settings->gatewayDescription);
		if (buildFile == nullptr)
		{
			errCount++;
			result = false;
		}
		else
		{
			m_cfgXml->addLinkToFile(buildFile);
		}

		buildFile = m_buildResultWriter->getBuildFileByID(Directory::COMMON, CfgFileId::APP_SIGNAL_SET);

		if (buildFile == nullptr)
		{
			errCount++;
			result = false;
		}
		else
		{
			m_cfgXml->addLinkToFile(buildFile);
		}

		QString resultStr = QString("Parsing of %1 gateway description finished with %2 errors, %3 warnings").
								arg(equipmentID()).arg(errCount).arg(wrnCount);
		if (errCount > 0)
		{
			log->writeError(resultStr);
		}
		else
		{
			if (wrnCount > 0)
			{
				log->writeWarning0(resultStr);
			}
			else
			{
				log->writeMessage(resultStr);
			}
		}

		return result;
	}

	bool GatewayServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														const GatewayServiceSettings& settings,
														E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgService1.address,
										   settings.cfgService2.address, os, true);

		if (cmdLine.isEmpty() == true)
		{
			return false;
		}

		content += cmdLine;

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);
		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
