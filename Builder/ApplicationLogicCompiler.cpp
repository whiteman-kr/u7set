#include "../lib/DeviceHelper.h"
#include "../lib/LmDescription.h"
#include "../lib/DataSource.h"
#include "../lib/ServiceSettings.h"

#include "ApplicationLogicCompiler.h"
#include "SoftwareCfgGenerator.h"
#include "BdfFile.h"

namespace Builder
{

	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//
	// ---------------------------------------------------------------------------------

	IssueLogger* ApplicationLogicCompiler::m_log = nullptr;


	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems,
													   const std::vector<Hardware::DeviceModule*>& lmModules,
													   Hardware::EquipmentSet *equipmentSet,
													   Hardware::OptoModuleStorage *optoModuleStorage,
													   Hardware::ConnectionStorage *connections,
													   SignalSet* signalSet,
													   LmDescriptionSet* lmDescriptions,
													   AppLogicData* appLogicData,
													   Tuning::TuningDataStorage* tuningDataStorage,
													   ComparatorStorage* comparatorStorage,
													   VFrame30::BusSet* busSet,
													   std::shared_ptr<BuildResultWriter> buildResultWriter,
													   IssueLogger *log,
													   bool expertMode) :
		m_subsystems(subsystems),
		m_lmModules(lmModules),
		m_equipmentSet(equipmentSet),
		m_optoModuleStorage(optoModuleStorage),
		m_signals(signalSet),
		m_lmDescriptions(lmDescriptions),
		m_appLogicData(appLogicData),
		m_tuningDataStorage(tuningDataStorage),
		m_cmpStorage(comparatorStorage),
		m_busSet(busSet),
		m_resultWriter(buildResultWriter),
		m_connections(connections),
		m_expertMode(expertMode)
	{
		if (m_log == nullptr)
		{
			m_log = log;
		}
		else
		{
			// only one instance of ApplicationLogicCompiler can be created !
			//
			assert(false);
		}

		if (m_equipmentSet != nullptr)
		{
			m_deviceRoot = m_equipmentSet->root();
		}
	}

	ApplicationLogicCompiler::~ApplicationLogicCompiler()
	{
		clear();
	}

	bool ApplicationLogicCompiler::run()
	{
		if (m_log == nullptr)
		{
			assert(m_log != nullptr);
			return false;
		}

		if (m_subsystems == nullptr ||
			m_equipmentSet == nullptr ||
			m_signals == nullptr ||
			m_lmDescriptions == nullptr ||
			m_appLogicData == nullptr ||
			m_tuningDataStorage == nullptr ||
			m_cmpStorage == nullptr ||
			m_resultWriter == nullptr ||
			m_connections == nullptr ||
			m_busSet == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		DeviceHelper::init();

		m_signals->resetAddresses();

		ApplicationLogicCompilerProc appLogicCompilerProcs[] =
		{
			&ApplicationLogicCompiler::prepareOptoConnectionsProcessing,
			&ApplicationLogicCompiler::checkLmIpAddresses,
			&ApplicationLogicCompiler::compileModulesLogicsPass1,
//			&ApplicationLogicCompiler::processBvbModules,
			&ApplicationLogicCompiler::compileModulesLogicsPass2,
			&ApplicationLogicCompiler::writeResourcesUsageReport,
			&ApplicationLogicCompiler::writeSerialDataXml,
			&ApplicationLogicCompiler::writeOptoConnectionsReport,
//			&ApplicationLogicCompiler::writeOptoModulesReport,
			&ApplicationLogicCompiler::writeOptoVhdFiles,
			&ApplicationLogicCompiler::writeAppSignalSetFile,
			&ApplicationLogicCompiler::writeSubsystemsXml,
		};

		bool result = true;

		int procsCount = sizeof(appLogicCompilerProcs) / sizeof(ApplicationLogicCompilerProc);

		for(int i = 0; i < procsCount; i++)
		{
			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}

			result &= (this->*appLogicCompilerProcs[i])();		// call next ApplicationLogicCompiler procedure

			if (result == false)
			{
				break;
			}
		}

		clear();

		DeviceHelper::shutdown();

		return result;
	}

	bool ApplicationLogicCompiler::isBuildCancelled()
	{
		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		return false;
	}

	bool ApplicationLogicCompiler::prepareOptoConnectionsProcessing()
	{
		if (m_optoModuleStorage == nullptr ||
			m_connections == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			ASSERT_RETURN_FALSE;
		}

		if (m_connections->count() == 0)
		{
			LOG_MESSAGE(m_log, QString(tr("No opto connections found")));
			LOG_SUCCESS(m_log, QString(tr("Ok")));
			return true;
		}

		if (m_optoModuleStorage->appendOptoModules() == false)
		{
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking opto connections")));

		if (m_optoModuleStorage->appendAndCheckConnections(*m_connections) == false)
		{
			return false;
		}

		return true;
	}

	bool ApplicationLogicCompiler::checkLmIpAddresses()
	{
		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Check LM's ethernet adapters IP addresses...")));

		bool result = true;

		QHash<QString, const Hardware::DeviceModule*> ip2Modules;

		for(const Hardware::DeviceModule* lm : m_lmModules)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			if (lm->isBvb() == true)
			{
				continue;
			}

			for(int i = 0; i < DataSource::LM_ETHERNET_ADAPTERS_COUNT; i++)
			{
				DataSource::LmEthernetAdapterProperties lmNetProperties;

				int ethernetAdapterNo = DataSource::LM_ETHERNET_ADAPTER1 + i;

				lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, ethernetAdapterNo, m_log);

				switch(ethernetAdapterNo)
				{
				case DataSource::LM_ETHERNET_ADAPTER1:
					// tuning data adapter
					//
					if (lmNetProperties.tuningEnable == true)
					{
						lmNetProperties.tuningIP = lmNetProperties.tuningIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.tuningIP) == true)
						{
							const Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.tuningIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.tuningIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.tuningIP, lm);
						}
					}
					break;

				case DataSource::LM_ETHERNET_ADAPTER2:
				case DataSource::LM_ETHERNET_ADAPTER3:
					// appllication and diagnostics data adapters
					//
					if (lmNetProperties.appDataEnable == true)
					{
						lmNetProperties.appDataIP = lmNetProperties.appDataIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.appDataIP) == true)
						{
							const Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.appDataIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.appDataIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.appDataIP, lm);
						}
					}

					if (lmNetProperties.diagDataEnable == true)
					{
						lmNetProperties.diagDataIP = lmNetProperties.diagDataIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.diagDataIP) == true)
						{
							const Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.diagDataIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.diagDataIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.diagDataIP, lm);
						}
					}

					break;

				default:
					assert(false);
				}
			}
		}

		if (result == true)
		{
			LOG_OK(m_log);
		}

		return result;
	}

	bool ApplicationLogicCompiler::compileModulesLogicsPass1()
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #1...")));

		bool result = true;

		// first compiler pass
		//
		for(const Hardware::DeviceModule* lm : m_lmModules)
		{
			if (lm == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			ModuleLogicCompiler* moduleLogicCompiler = new ModuleLogicCompiler(*this, lm, m_expertMode);

			m_moduleCompilers.append(moduleLogicCompiler);

			result &= moduleLogicCompiler->pass1();

			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}
		}

		return result;
	}

	bool ApplicationLogicCompiler::processBvbModules()
	{
		return m_optoModuleStorage->processBvbModules();
	}

	bool ApplicationLogicCompiler::compileModulesLogicsPass2()
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #2...")));

		// second compiler pass
		//
		for(ModuleLogicCompiler* moduleLogicCompiler : m_moduleCompilers)
		{
			if (moduleLogicCompiler == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			result &= moduleLogicCompiler->pass2();

			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}
		}

		return result;
	}

	bool ApplicationLogicCompiler::writeResourcesUsageReport()
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Resources usage report generation...")));

		QList<std::tuple<QString, QString, double, double, double, double, double>> fileContent;
		QStringList fileContentStringList;

		QString header = "LM Equipment ID";

		QStringList restHeaderColumns;
		restHeaderColumns << "Bit Memory, %"
						  << "Word Memory, %"
						  << "Code memory, %"
						  << "IdrPhase Time, %"
						  << "AlpPhase Time, %";

		int maxIdLength = header.length();

		QStringList afbsUsage;

		QStringList afbsUsageHeader;
		afbsUsageHeader << "OpCode"
						<< "Caption"
						<< "UsagePercent"
						<< "UsedInstances"
						<< "MaxInstances"
						<< "Version";

		auto getFirstFieldValue = [](double value) -> QString
		{
			if (value > 100)
			{
				return "##";
			}

			if (value > 90)
			{
				return "!!";
			}

			if (value > 80)
			{
				return "! ";
			}

			return "  ";
		};

		for(int i = 0; i < m_moduleCompilers.count(); i++)
		{
			ModuleLogicCompiler* moduleCompiler = m_moduleCompilers[i];

			ModuleLogicCompiler::ResourcesUsageInfo info = moduleCompiler->resourcesUsageInfo();

			double maxValue = info.bitMemoryUsed;
			maxValue = std::max(maxValue, info.wordMemoryUsed);
			maxValue = std::max(maxValue, info.codeMemoryUsed);
			maxValue = std::max(maxValue, info.idrPhaseTimeUsed);
			maxValue = std::max(maxValue, info.alpPhaseTimeUsed);

			QString firstFieldValue = getFirstFieldValue(maxValue);

			maxIdLength = std::max(maxIdLength, info.lmEquipmentID.length());

			fileContent << std::make_tuple(firstFieldValue,
										   info.lmEquipmentID,
										   info.bitMemoryUsed,
										   info.wordMemoryUsed,
										   info.codeMemoryUsed,
										   info.idrPhaseTimeUsed,
										   info.alpPhaseTimeUsed);

			// creating AFBL Usage tables
			//
			if (info.afblUsageInfo.count() == 0)
			{
				continue;
			}

			int captionLength = afbsUsageHeader[1].length();
			for(const ModuleLogicCompiler::AfblUsageInfo& afblUsage : info.afblUsageInfo)
			{
				captionLength = std::max(captionLength, afblUsage.caption.length());
			}

			afbsUsage.append("");
			afbsUsage.append(QString("LM %1 AFB components usage").arg(info.lmEquipmentID));
			afbsUsage.append("");

			QString headerRow = "  ";
			QString delimiterRow = "--";

			for (int j = 0; j < afbsUsageHeader.count(); j++)
			{
				int length = (j == 1) ? captionLength : afbsUsageHeader[j].length();
				headerRow += " | " + afbsUsageHeader[j].leftJustified(length);
				delimiterRow += "-+-" + QString().leftJustified(length, '-');
			}

			afbsUsage << headerRow << delimiterRow;

			for(const ModuleLogicCompiler::AfblUsageInfo& afblUsage : info.afblUsageInfo)
			{
				afbsUsage << getFirstFieldValue(afblUsage.usagePercent) + " | " +
							 QString::number(afblUsage.opCode).rightJustified(afbsUsageHeader[0].length()) + " | " +
						afblUsage.caption.leftJustified(captionLength) + " | " +
						QString::number(afblUsage.usagePercent, 'f', 2).rightJustified(afbsUsageHeader[2].length()) + " | " +
						QString::number(afblUsage.usedInstances).rightJustified(afbsUsageHeader[3].length()) + " | " +
						QString::number(afblUsage.maxInstances).rightJustified(afbsUsageHeader[4].length()) + " | " +
						QString::number(afblUsage.version).rightJustified(afbsUsageHeader[5].length());
			}

			afbsUsage.append("");
			// creating AFBL Usage tables complete
		}

		auto reportGenerator = [&](QString caption, std::function<bool
				(std::tuple<QString, QString, double, double, double, double, double>& first,
				 std::tuple<QString, QString, double, double, double, double, double>& second)> comparator,
				int expectedRowQuantity = -1)
		{
			QStringList result;

			result << caption;
			result << "";

			std::sort(fileContent.begin(), fileContent.end(), comparator);

			if (expectedRowQuantity == -1)
			{
				expectedRowQuantity = fileContent.count();
			}
			int reportRowQuantity = std::min(expectedRowQuantity, fileContent.count());

			result << "   | " + header.leftJustified(maxIdLength, ' ') + " | " + restHeaderColumns.join(" | ");

			QString delimiter;
			delimiter = "---+-" + delimiter.leftJustified(maxIdLength, '-');

			for (int i = 0; i < restHeaderColumns.count(); i++)
			{
				delimiter += "-+-";
				delimiter = delimiter.leftJustified(delimiter.length() + restHeaderColumns[i].length(), '-');
			}

			result << delimiter;

			for (int i = 0; i < reportRowQuantity; i++)
			{
				auto& item = fileContent[i];
				result << std::get<0>(item) + " | " +
						  std::get<1>(item).leftJustified(maxIdLength, ' ') + " | " +
						  QString::number(std::get<2>(item), 'f', 2).rightJustified(restHeaderColumns[0].length()) + " | " +
						  QString::number(std::get<3>(item), 'f', 2).rightJustified(restHeaderColumns[1].length()) + " | " +
						  QString::number(std::get<4>(item), 'f', 2).rightJustified(restHeaderColumns[2].length()) + " | " +
						  QString::number(std::get<5>(item), 'f', 2).rightJustified(restHeaderColumns[3].length()) + " | " +
						  QString::number(std::get<6>(item), 'f', 2).rightJustified(restHeaderColumns[4].length());
			}

			result << "" << "";

			return result;
		};

		fileContentStringList = reportGenerator("LM's resources usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<1>(first) < std::get<1>(second);
		});

		fileContentStringList << reportGenerator("Top LMs of BitMemory usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<2>(first) > std::get<2>(second);
		}, 10);

		fileContentStringList << reportGenerator("Top LMs of WordMemory usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<3>(first) > std::get<3>(second);
		}, 10);

		fileContentStringList << reportGenerator("Top LMs of CodeMemory usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<4>(first) > std::get<4>(second);
		}, 10);

		fileContentStringList << reportGenerator("Top LMs of IdrPhase Time usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<5>(first) > std::get<5>(second);
		}, 10);

		fileContentStringList << reportGenerator("Top LMs of AlpPhase Time usage", []
												(std::tuple<QString, QString, double, double, double, double, double>& first,
												std::tuple<QString, QString, double, double, double, double, double>& second)
		{
			return std::get<6>(first) > std::get<6>(second);
		}, 10);

		fileContentStringList.append("");
		fileContentStringList.append(afbsUsage);

		m_resultWriter->addFile(Builder::DIR_REPORTS, "Resources.txt", fileContentStringList);

		return result;
	}

	bool ApplicationLogicCompiler::writeBinCodeForLm(QString subsystemID,
													 int subsystemKey,
													 int appLogicUartId,
													 QString lmEquipmentID,
													 int lmNumber,
													 int frameSize,
													 int frameCount,
													 quint64 uniqueID,
													 const QString& lmDesctriptionFile,
													 int lmDescriptionNumber,
													 ApplicationLogicCode& appLogicCode)
	{
		if (m_resultWriter == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		bool result = true;

		int metadataFieldsVersion = 0;

		QStringList metadataFields;

		appLogicCode.getAsmMetadataFields(&metadataFields, &metadataFieldsVersion);

		Hardware::ModuleFirmwareWriter* firmwareWriter = m_resultWriter->firmwareWriter();

		if (firmwareWriter == nullptr)
		{
			assert(firmwareWriter);
			return false;
		}

		firmwareWriter->createFirmware(subsystemID,
									   subsystemKey,
									   appLogicUartId,
									   "AppLogic",
									   frameSize,
									   frameCount,
									   lmDesctriptionFile,
									   lmDescriptionNumber);

		firmwareWriter->setDescriptionFields(subsystemID, appLogicUartId, metadataFieldsVersion, metadataFields);

		QByteArray binCode;

		appLogicCode.getBinCode(&binCode);

		std::vector<QVariantList> metadata;

		appLogicCode.getAsmMetadata(&metadata);

		result &= firmwareWriter->setChannelData(subsystemID, appLogicUartId, lmEquipmentID, lmNumber, frameSize, frameCount, uniqueID, binCode, metadata, m_log);

		return result;
	}

	bool ApplicationLogicCompiler::writeSerialDataXml()
	{
		return m_optoModuleStorage->writeSerialDataXml(m_resultWriter.get());
	}

	bool ApplicationLogicCompiler::writeOptoConnectionsReport()
	{
		int count = m_connections->count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "==================================================================================";
		QString delim2 = "----------------------------------------------------------------------------------";

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Hardware::Connection> cn = m_connections->get(i);

			if (cn == nullptr)
			{
				assert(false);
				continue;
			}

			list.append(delim);

			list.append(QString(tr("Connection ID:\t\t\t%1")).arg(cn->connectionID()));
			list.append(QString(tr("Link ID:\t\t\t%1\n")).arg(cn->linkID()));
			list.append(QString(tr("Mode:\t\t\t\t%1")).arg(cn->typeStr()));
			list.append(QString(tr("Settings:\t\t\t%1")).arg(cn->manualSettings() == true ? "Manual" : "Auto"));
			list.append(QString(tr("Data ID control:\t\t%1\n")).arg(cn->disableDataId() == true ? "Disabled" : "Enabled"));

			list.append(QString(tr("Port1 equipmentID:\t\t%1")).arg(cn->port1EquipmentID()));

			if (cn->isPortToPort() == true)
			{
				list.append(QString(tr("Port2 equipmentID:\t\t%1")).arg(cn->port2EquipmentID()));
			}

			list.append(delim);
			list.append("");

			switch(cn->type())
			{
			case Hardware::Connection::Type::SinglePort:
				{
					Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

					if (p1 != nullptr)
					{
						p1->writeInfo(list);
					}
					else
					{
						assert(false);
					}
				}
				break;

			case Hardware::Connection::Type::PortToPort:
				{
					Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

					if (p1 != nullptr)
					{
						p1->writeInfo(list);
					}
					else
					{
						assert(false);
					}

					list.append(delim2);
					list.append("");

					Hardware::OptoPortShared p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

					if (p2 != nullptr)
					{
						p2->writeInfo(list);
					}
					else
					{
						assert(false);
					}
				}
				break;

			default:
				assert(false);
				return false;
			}
		}

		m_resultWriter->addFile(Builder::DIR_REPORTS, "Connections.txt", "", "", list);

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoVhdFiles()
	{
		int count = m_connections->count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "--------------------------------------------------------------------";

		QString str;

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Hardware::Connection> cn = m_connections->get(i);

			if (cn == nullptr)
			{
				assert(false);
				continue;
			}

			if (cn->generateVHDFile() == false)
			{
				continue;
			}

			if (cn->isPortToPort() == true)
			{
				Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
				Hardware::OptoPortShared p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

				writeOptoPortToPortVhdFile(cn->connectionID(), p1, p2);
				writeOptoPortToPortVhdFile(cn->connectionID(), p2, p1);
			}
			else
			{
				Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
				writeOptoSinglePortVhdFile(cn->connectionID(), p1);
			}
		}

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoPortToPortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort)
	{
		if (outPort == nullptr || inPort == nullptr)
		{
			assert(false);
			return false;
		}

		if (outPort->txSignalsCount() == 0)
		{
			return true;
		}

		QString outPortID = outPort->equipmentID().toLower();
		QString inPortID = inPort->equipmentID().toLower();

		QString vhdFileName = QString("%1.vhd").arg(inPortID);
		QString bdfFileName = QString("%1.bdf").arg(inPortID);

		BdfFile bdfFile;

		QStringList list;
		QString str;

		int inBusWidth = outPort->txDataSizeW() * SIZE_16BIT;

		quint32 dataID = outPort->txDataID();

		QVector<Hardware::TxRxSignalShared> txAnalogs;

		outPort->getTxAnalogSignals(txAnalogs, false);

		QVector<Hardware::TxRxSignalShared> txDiscretes;

		outPort->getTxDiscreteSignals(txDiscretes, false);

		list.append("--");
		list.append("-- This file has been generated automatically by RPCT software");
		list.append("--");

		BuildInfo bi = m_resultWriter->buildInfo();

		str = QString("-- Project:\t%1").arg(bi.project);
		list.append(str);

		str = QString("-- Build No:\t%1").arg(bi.id);
		list.append(str);

		str = QString("-- Build type:\t%1").arg(bi.release == true ? "Release" : "Debug");
		list.append(str);

		str = QString("-- Build date:\t%1").arg(bi.dateStr());
		list.append(str);

		str = QString("-- User:\t%1").arg(bi.user);
		list.append(str);

		str = QString("-- Host:\t%1").arg(bi.workstation);
		list.append(str);

		list.append("--");

		str = QString("-- Port-to-port connection ID:\t%1").arg(connectionID);
		list.append(str);

		str = QString("-- Opto port ID:\t%1").arg(inPort->equipmentID());
		list.append(str);

		str = QString("-- Rx data size:\t%1 words (%2 bytes)").arg(outPort->txDataSizeW()).arg(outPort->txDataSizeW() * sizeof(quint16));
		list.append(str);

		str.sprintf("-- Rx data ID:\t\t%u (0x%08X)", dataID, dataID);
		list.append(str);

		list.append("--\n");

		// declaration section

		list.append("library ieee;");
		list.append("use ieee.std_logic_1164.all;");
		list.append("use ieee.numeric_std.all;\n");

		str = QString("entity %1 is\n\tport (\n").arg(inPortID);
		list.append(str);

		str = QString("\t\tconst_rx_data_id : out std_logic_vector(32-1 downto 0);");
		list.append(str);

		bdfFile.addConnector32("const_rx_data_id");

		str = QString("\t\trx_data_id : out std_logic_vector(32-1 downto 0);\n");
		list.append(str);

		bdfFile.addConnector32("rx_data_id");

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t\t%1 : out std_logic_vector(%2-1 downto 0);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->dataSize());

				list.append(str);

				bdfFile.addConnector(txAnalog->appSignalID(), txAnalog->dataSize());
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t\t%1 : out std_logic;").arg(txDiscrete->appSignalID().remove("#"));
				list.append(str);

				bdfFile.addConnector1(txDiscrete->appSignalID());
			}

			list.append("");
		}

		str = QString("\t\t%1 : in std_logic_vector(%2-1 downto 0)").arg(outPortID).arg(inBusWidth);
		list.append(str);

		str = QString("\t);\nend %1;\n").arg(inPortID);
		list.append(str);

		// architecture section

		str = QString("architecture arch of %1 is").arg(inPortID);
		list.append(str);

		str = QString("\tsignal in_data : std_logic_vector(%1-1 downto 0);").arg(inBusWidth);
		list.append(str);

		str = QString("begin");
		list.append(str);

		str = QString("\tin_data <= %1;\n").arg(outPortID);
		list.append(str);

		str = QString("\tconst_rx_data_id <= std_logic_vector(to_unsigned(%1,32));").arg(dataID);
		list.append(str);
		str = QString("\trx_data_id <= in_data(32-1 downto 0);\n");
		list.append(str);

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t%1 <= in_data(%2-1 downto %3);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->addrInBuf().offset() * 16 + txAnalog->dataSize()).
						arg(txAnalog->addrInBuf().offset() * 16);

				list.append(str);
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t%1 <= in_data(%2);").
						arg(txDiscrete->appSignalID().remove("#")).
						arg(txDiscrete->addrInBuf().offset() * 16 + txDiscrete->addrInBuf().bit());
				list.append(str);
			}

			list.append("");
		}

		list.append("end arch;");

		m_resultWriter->addFile(Builder::DIR_OPTO_VHD, vhdFileName, list);

		m_resultWriter->addFile(Builder::DIR_OPTO_VHD, bdfFileName, bdfFile.stringList());

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoSinglePortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort)
	{
		if (outPort == nullptr)
		{
			assert(false);
			return false;
		}

		if (outPort->txSignalsCount() == 0)
		{
			return true;
		}

		QString outPortID = outPort->equipmentID().toLower();

		QString vhdFileName = QString("%1_tx.vhd").arg(outPortID);
		QString bdfFileName = QString("%1_tx.bdf").arg(outPortID);

		BdfFile bdfFile;

		QStringList list;
		QString str;

		int inBusWidth = outPort->txDataSizeW() * SIZE_16BIT;

		quint32 dataID = outPort->txDataID();

		QVector<Hardware::TxRxSignalShared> txAnalogs;

		outPort->getTxAnalogSignals(txAnalogs, false);

		QVector<Hardware::TxRxSignalShared> txDiscretes;

		outPort->getTxDiscreteSignals(txDiscretes, false);

		list.append("--");
		list.append("-- This file has been generated automatically by RPCT software");
		list.append("--");

		BuildInfo bi = m_resultWriter->buildInfo();

		str = QString("-- Project:\t%1").arg(bi.project);
		list.append(str);

		str = QString("-- Build No:\t%1").arg(bi.id);
		list.append(str);

		str = QString("-- Build type:\t%1").arg(bi.release == true ? "Release" : "Debug");
		list.append(str);

		str = QString("-- Build date:\t%1").arg(bi.dateStr());
		list.append(str);

		str = QString("-- User:\t%1").arg(bi.user);
		list.append(str);

		str = QString("-- Host:\t%1").arg(bi.workstation);
		list.append(str);

		list.append("--");

		str = QString("-- Single-port connection ID:\t%1").arg(connectionID);
		list.append(str);

		str = QString("-- Rx data size:\t%1 words (%2 bytes)").arg(outPort->txDataSizeW()).arg(outPort->txDataSizeW() * sizeof(quint16));
		list.append(str);

		str.sprintf("-- Rx data ID:\t\t%u (0x%08X)", dataID, dataID);
		list.append(str);

		list.append("--\n");

		// declaration section

		list.append("library ieee;");
		list.append("use ieee.std_logic_1164.all;");
		list.append("use ieee.numeric_std.all;\n");

		str = QString("entity %1_tx is\n\tport (\n").arg(outPortID);
		list.append(str);

		str = QString("\t\tconst_rx_data_id : out std_logic_vector(32-1 downto 0);");
		list.append(str);

		bdfFile.addConnector32("const_rx_data_id");

		str = QString("\t\trx_data_id : out std_logic_vector(32-1 downto 0);\n");
		list.append(str);

		bdfFile.addConnector32("rx_data_id");

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t\t%1 : out std_logic_vector(%2-1 downto 0);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->dataSize());

				list.append(str);

				bdfFile.addConnector(txAnalog->appSignalID(), txAnalog->dataSize());
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t\t%1 : out std_logic;").arg(txDiscrete->appSignalID().remove("#"));
				list.append(str);

				bdfFile.addConnector1(txDiscrete->appSignalID());
			}

			list.append("");
		}

		str = QString("\t\t%1 : in std_logic_vector(%2-1 downto 0)").arg(outPortID).arg(inBusWidth);
		list.append(str);

		str = QString("\t);\nend %1_tx;\n").arg(outPortID);
		list.append(str);

		// architecture section

		str = QString("architecture arch of %1_tx is").arg(outPortID);
		list.append(str);

		str = QString("\tsignal in_data : std_logic_vector(%1-1 downto 0);").arg(inBusWidth);
		list.append(str);

		str = QString("begin");
		list.append(str);

		str = QString("\tin_data <= %1;\n").arg(outPortID);
		list.append(str);

		str = QString("\tconst_rx_data_id <= std_logic_vector(to_unsigned(%1,32));").arg(dataID);
		list.append(str);
		str = QString("\trx_data_id <= in_data(32-1 downto 0);\n");
		list.append(str);

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t%1 <= in_data(%2-1 downto %3);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->addrInBuf().offset() * 16 + txAnalog->dataSize()).
						arg(txAnalog->addrInBuf().offset() * 16);

				list.append(str);
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t%1 <= in_data(%2);").
						arg(txDiscrete->appSignalID().remove("#")).
						arg(txDiscrete->addrInBuf().offset() * 16 + txDiscrete->addrInBuf().bit());
				list.append(str);
			}

			list.append("");
		}

		list.append("end arch;");

		m_resultWriter->addFile(Builder::DIR_OPTO_VHD, vhdFileName, list);

		m_resultWriter->addFile(Builder::DIR_OPTO_VHD, bdfFileName, bdfFile.stringList());

		return true;
	}



	bool ApplicationLogicCompiler::writeOptoModulesReport()
	{
		QVector<Hardware::OptoModuleShared> modules;

		m_optoModuleStorage->getOptoModulesSorted(modules);

		int count = modules.count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "--------------------------------------------------------------------";

		QString str;

		for(int i = 0; i < count; i++)
		{
			Hardware::OptoModuleShared module = modules[i];

			if (module == nullptr)
			{
				assert(false);
				continue;
			}

			list.append(delim);

			if (module->isLmOrBvb())
			{
				str = QString(tr("Opto module LM (or BVB) %1")).arg(module->equipmentID());
			}
			else
			{
				if (module->isOcm())
				{
					str = QString(tr("Opto module OCM %1")).arg(module->equipmentID());
				}
				else
				{
					assert(false);
				}
			}

			list.append(str);
			list.append(delim);
			list.append("");

			// write module's opto ports information
			//
			const HashedVector<QString, Hardware::OptoPortShared>& ports = module->ports();

			for(const Hardware::OptoPortShared& port : ports)
			{
				port->writeInfo(list);
			}
		}

		m_resultWriter->addFile(Builder::DIR_REPORTS, "Opto-modules.txt", "", "", list);

		return true;
	}

	bool ApplicationLogicCompiler::writeAppSignalSetFile()
	{
		if (m_signals == nullptr)
		{
			assert(false);
			return false;
		}

		::Proto::AppSignalSet protoAppSignalSet;

		// fill signals
		//
		int signalCount = m_signals->count();

		for(int i = 0; i < signalCount; i++)
		{
			const Signal& s = (*m_signals)[i];

			::Proto::AppSignal* protoAppSignal = protoAppSignalSet.add_appsignal();

			s.serializeTo(protoAppSignal);
		}

		int dataSize = protoAppSignalSet.ByteSize();

		QByteArray data;

		data.resize(dataSize);

		protoAppSignalSet.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(data.data()));

		BuildFile* appSignalSetFile = m_resultWriter->addFile(Builder::DIR_COMMON, FILE_APP_SIGNALS_ASGS, CFG_FILE_ID_APP_SIGNAL_SET, "", data, true);

		return appSignalSetFile != nullptr;
	}

	bool ApplicationLogicCompiler::writeSubsystemsXml()
	{
		bool result = true;

		int subsystemsCount = m_subsystems->count();

		QHash<QString, std::shared_ptr<Hardware::Subsystem>> subsystems;

		for(int i = 0; i < subsystemsCount; i++)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);

			if (subsystem == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			subsystems.insert(subsystem->subsystemId(), subsystem);
		}

		QHash<QString, QString> subsystemModules;
		QHash<QString, const Hardware::DeviceModule*> modules;

		for(const Hardware::DeviceModule* module : m_lmModules)
		{
			if (module == nullptr)
			{
				LOG_NULLPTR_ERROR(m_log);
				result = false;
				continue;
			}

			modules.insert(module->equipmentIdTemplate(), module);

			QString lmSubsystem;

			bool res= DeviceHelper::getStrProperty(module, "SubsystemID", &lmSubsystem, m_log);

			if (res == false)
			{
				result = false;
				continue;
			}

			if (subsystems.contains(lmSubsystem) == false)
			{
				// Subsystem '%1' is not found in subsystem set (Logic Module '%2').
				m_log->errCFG3001(lmSubsystem, module->equipmentIdTemplate());
				result = false;
				continue;
			}

			subsystemModules.insertMulti(lmSubsystem, module->equipmentIdTemplate());
		}

		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();

		m_resultWriter->buildInfo().writeToXml(*xml.xmlStreamWriter());

		xml.writeStartElement("Subsystems");
		xml.writeIntAttribute("Count", subsystemsCount);

		QStringList subsystemIDs = subsystems.uniqueKeys();

		subsystemIDs.sort();

		for(const QString& subsystemID : subsystemIDs)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = subsystems.value(subsystemID, nullptr);

			if (subsystem == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			QStringList subsysModuleIds = subsystemModules.values(subsystemID);
			subsysModuleIds.sort();

			xml.writeStartElement("Subsystem");

			xml.writeStringAttribute("Id", subsystem->subsystemId());
			xml.writeStringAttribute("Caption", subsystem->caption());
			xml.writeIntAttribute("Index", subsystem->index());
			xml.writeIntAttribute("Key", subsystem->key());
			xml.writeIntAttribute("ModulesCount", subsysModuleIds.count());

			for(const QString& moduleID : subsysModuleIds)
			{
				const Hardware::DeviceModule* module = modules.value(moduleID, nullptr);

				if (module == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				int lmNumber = 0;
				int lmChannel = 0;
				QString lmSubsystem = 0;

				bool res = true;

				res &= DeviceHelper::getIntProperty(module, "LMNumber", &lmNumber, m_log);
				res &= DeviceHelper::getIntProperty(module, "SubsystemChannel", &lmChannel, m_log);
				res &= DeviceHelper::getStrProperty(module, "SubsystemID", &lmSubsystem, m_log);

				if (res == false)
				{
					result = false;
					continue;
				}

				xml.writeStartElement("Module");

				xml.writeStringAttribute("EquipmentId", module->equipmentIdTemplate());
				xml.writeStringAttribute("SubsystemId", lmSubsystem);
				xml.writeIntAttribute("LmNumber", lmNumber);
				xml.writeIntAttribute("SubsystemChannel", lmChannel);
				xml.writeIntAttribute("ModuleType", module->moduleType());
				xml.writeIntAttribute("ModuleFamily", module->moduleFamily());
				xml.writeIntAttribute("ModuleVersion", module->moduleVersion());
				xml.writeIntAttribute("CustomModuleFamily", module->customModuleFamily());

				xml.writeEndElement(); // </Module>
			}

			xml.writeEndElement(); // </Subsystem>
		}

		xml.writeEndElement(); // </Subsystems>
		xml.writeEndDocument();

		BuildFile* buildFile = m_resultWriter->addFile(Builder::DIR_COMMON, "Subsystems.xml", "", "",  data);

		if (buildFile == nullptr)
		{
			result = false;
		}

		return result;
	}

	const LmDescriptionSet& ApplicationLogicCompiler::lmDescriptionSet() const
	{
		assert(m_lmDescriptions);
		return *m_lmDescriptions;
	}

	void ApplicationLogicCompiler::clear()
	{
		for(ModuleLogicCompiler* mc : m_moduleCompilers)
		{
			delete mc;
		}

		m_moduleCompilers.clear();

		m_log = nullptr;
	}

}



