#include "ApplicationLogicCompiler.h"
#include "SoftwareCfgGenerator.h"

namespace Builder
{

	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//
	// ---------------------------------------------------------------------------------


	IssueLogger* ApplicationLogicCompiler::m_log = nullptr;


	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems,
													   Hardware::EquipmentSet *equipmentSet,
													   Hardware::OptoModuleStorage *optoModuleStorage,
													   Hardware::ConnectionStorage *connections,
													   SignalSet* signalSet,
													   Afb::AfbElementCollection *afblSet,
													   AppLogicData* appLogicData,
													   Tuning::TuningDataStorage* tuningDataStorage,
													   BuildResultWriter* buildResultWriter,
													   IssueLogger *log) :
		m_subsystems(subsystems),
		m_equipmentSet(equipmentSet),
		m_optoModuleStorage(optoModuleStorage),
		m_signals(signalSet),
		m_afbl(afblSet),
		m_appLogicData(appLogicData),
		m_tuningDataStorage(tuningDataStorage),
		m_resultWriter(buildResultWriter),
		m_connections(connections)
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


	void ApplicationLogicCompiler::clear()
	{
		for(ModuleLogicCompiler* mc : m_moduleCompilers)
		{
			delete mc;
		}

		m_moduleCompilers.clear();

		m_log = nullptr;
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
			m_afbl == nullptr ||
			m_appLogicData == nullptr ||
			m_tuningDataStorage == nullptr ||
			m_resultWriter == nullptr ||
			m_connections == nullptr)
		{
			msg = tr("%1: Invalid params. Compilation aborted.").arg(__FUNCTION__);

			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

			qDebug() << msg;

			return false;
		}

		m_signals->resetAddresses();

		findLMs();

		bool result = false;

		do
		{
			if (checkAppSignals() == false) break;

			if (checkOptoConnections() == false) break;

			if (checkLmIpAddresses() == false) break;

			if (compileModulesLogicsPass1() == false) break;

			if (disposeOptoModulesTxRxBuffers() == false) break;

			if (writeOptoConnectionsReport() == false) break;

			if (writeOptoModulesReport() == false) break;

			if (writeOptoVhdFiles() == false) break;

			if (compileModulesLogicsPass2() == false) break;

			result = true;

			break;

		} while(true);

		clear();

		return result;
	}


	bool ApplicationLogicCompiler::checkAppSignals()
	{
		bool result = true;

		int count = m_signals->count();

		if (count == 0)
		{
			return true;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking application signals")));

		QHash<QString, int> appSignalIDs;

		appSignalIDs.reserve(count * 1.3);

		for(int i = 0; i < count; i++)
		{
			const Signal& s = (*m_signals)[i];

			// check AppSignalID
			//
			if (appSignalIDs.contains(s.appSignalID()) == true)
			{
				m_log->errALC5016(s.appSignalID());		// Application signal identifier '%1' is not unique.
				result = false;
			}
			else
			{
				appSignalIDs.insert(s.appSignalID(), i);
			}

			// check EquipmentID
			//
			if (s.equipmentID().isEmpty() == true)
			{
				m_log->wrnALC5012(s.appSignalID());		// Application signal '%1' is not bound to any device object.
			}
			else
			{
				Hardware::DeviceObject* device = m_equipmentSet->deviceObject(s.equipmentID());

				if (device == nullptr)
				{
					m_log->errALC5013(s.appSignalID(), s.equipmentID());		// Application signal '%1' is bound to unknown device object '%2'.
					result = false;
				}

				bool deviceOK = false;

				if (device->isModule())
				{
					Hardware::DeviceModule* module = device->toModule();

					if (module != nullptr && module->isLM())
					{
						deviceOK = true;
					}
				}
				else
				{
					if (device->isSignal())
					{
						deviceOK = true;
					}
				}

				if (deviceOK == false)
				{
					// The signal '%1' can be bind only to Logic Module or Equipment Signal.
					//
					m_log->errALC5031(s.appSignalID());
					result = false;
					continue;
				}
			}

			// check other signal properties
			//
			if (s.isDiscrete())
			{
				if (s.dataSize() != 1)
				{
					m_log->errALC5014(s.appSignalID());		// Discrete signal '%1' must have DataSize equal to 1.
					result = false;
				}
			}
			else
			{
				assert(s.isAnalog() == true);

				if (s.dataSize() != 32)
				{
					m_log->errALC5015(s.appSignalID());		// Analog signal '%1' must have DataSize equal to 32.
					result = false;
				}
			}
		}

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Ok")))
		}

		return result;
	}


	bool ApplicationLogicCompiler::checkOptoConnections()
	{
		if (m_optoModuleStorage == nullptr ||
			m_connections == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (m_optoModuleStorage->build() == false)
		{
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking opto connections")));

		if (m_optoModuleStorage->addConnections(*m_connections) == false)
		{
			return false;
		}

		int connectionsCount = m_connections->count();

		if (connectionsCount == 0)
		{
			LOG_MESSAGE(m_log, QString(tr("No opto connections found")));
			LOG_SUCCESS(m_log, QString(tr("Ok")));
			return true;
		}

		bool result = true;

		for(int i = 0; i < connectionsCount; i++)
		{
			std::shared_ptr<Hardware::Connection> connection = m_connections->get(i);

			if (connection->manualSettings() == true)
			{
				m_log->wrnALC5055(connection->connectionID());
			}

			quint16 portID = connection->getID();

			Hardware::OptoPort* optoPort1 = nullptr;

			optoPort1 = m_optoModuleStorage->getOptoPort(connection->port1EquipmentID());

			// check port 1
			//
			if (optoPort1 == nullptr)
			{
				// Undefined opto port '%1' in the connection '%2'.
				//
				m_log->errALC5021(connection->port1EquipmentID(), connection->connectionID());
				result = false;
				continue;
			}

			if (optoPort1->connectionID().isEmpty() == true)
			{
				optoPort1->setConnectionID(connection->connectionID());
			}
			else
			{
				// Opto port '%1' of connection '%2' is already used in connection '%3'.
				//
				m_log->errALC5019(optoPort1->equipmentID(), connection->connectionID(), optoPort1->connectionID());
				result = false;
				continue;
			}

			Hardware::OptoModule* optoModule = m_optoModuleStorage->getOptoModule(optoPort1);

			if (optoModule == nullptr)
			{
				assert(false);
				result = false;
				continue;
			}

			if (connection->mode() == Hardware::OptoPort::Mode::Serial)
			{
				optoPort1->setPortID(portID);
				optoPort1->setMode(Hardware::OptoPort::Mode::Serial);
				optoPort1->setSerialMode(connection->serialMode());
				optoPort1->setEnableSerial(connection->enableSerial());
				optoPort1->setEnableDuplex(connection->enableDuplex());

				optoPort1->setManualSettings(connection->manualSettings());
				optoPort1->setManualTxStartAddressW(connection->port1ManualTxStartAddress());
				optoPort1->setManualTxSizeW(connection->port1ManualTxWordsQuantity());
				optoPort1->setManualRxSizeW(connection->port1ManualRxWordsQuantity());

				if (optoModule->deviceModule()->moduleFamily() == Hardware::DeviceModule::FamilyType::LM)
				{
					// LM's port '%1' can't work in RS232/485 mode (connection '%2').
					//
					m_log->errALC5020(connection->port1EquipmentID(), connection->connectionID());
					result = false;
				}

				LOG_MESSAGE(m_log, QString(tr("RS232/485 connection '%1' ID = %2... Ok")).
							arg(connection->connectionID()).arg(portID));
			}
			else
			{
				assert(connection->mode() == Hardware::OptoPort::Mode::Optical);

				// check port 2
				//
				Hardware::OptoPort* optoPort2 = m_optoModuleStorage->getOptoPort(connection->port2EquipmentID());

				if (optoPort2 == nullptr)
				{
					// Undefined opto port '%1' in the connection '%2'.
					//
					m_log->errALC5021(connection->port2EquipmentID(), connection->connectionID());
					result = false;
					continue;
				}

				Hardware::OptoModule* m1 = m_optoModuleStorage->getOptoModule(optoPort1);
				Hardware::OptoModule* m2 = m_optoModuleStorage->getOptoModule(optoPort2);

				if (m1 != nullptr && m2 != nullptr)
				{
					if (m1->lmID() == m2->lmID())
					{
						//  Opto ports of the same chassis is linked via connection '%1'.
						//
						m_log->errALC5022(connection->connectionID());
						result = false;
						continue;
					}
				}
				else
				{
					assert(false);
				}

				if (optoPort2->connectionID().isEmpty() == true)
				{
					optoPort2->setConnectionID(connection->connectionID());
				}
				else
				{
					// Opto port '%1' of connection '%2' is already used in connection '%3'.
					//
					m_log->errALC5019(optoPort2->equipmentID(), connection->connectionID(), optoPort2->connectionID());
					result = false;
					continue;
				}

				optoPort1->setPortID(portID);
				optoPort1->setMode(Hardware::OptoPort::Mode::Optical);
				optoPort1->setManualSettings(connection->manualSettings());
				optoPort1->setManualTxStartAddressW(connection->port1ManualTxStartAddress());
				optoPort1->setManualTxSizeW(connection->port1ManualTxWordsQuantity());
				optoPort1->setManualRxSizeW(connection->port1ManualRxWordsQuantity());

				optoPort2->setPortID(portID);
				optoPort2->setMode(Hardware::OptoPort::Mode::Optical);
				optoPort2->setManualSettings(connection->manualSettings());
				optoPort2->setManualTxStartAddressW(connection->port2ManualTxStartAddress());
				optoPort2->setManualTxSizeW(connection->port2ManualTxWordsQuantity());
				optoPort2->setManualRxSizeW(connection->port2ManualRxWordsQuantity());

				optoPort1->setLinkedPortID(optoPort2->equipmentID());
				optoPort2->setLinkedPortID(optoPort1->equipmentID());

				LOG_MESSAGE(m_log, QString(tr("Optical connection '%1' ID = %2... Ok")).
							arg(connection->connectionID()).arg(portID));
			}
		}

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Ok")));
		}

		return result;
	}


	bool ApplicationLogicCompiler::checkLmIpAddresses()
	{
		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Check LM's ethernet adapters IP addresses...")));

		bool result = true;

		QHash<QString, Hardware::DeviceModule*> ip2Modules;

		for(Hardware::DeviceModule* lm : m_lm)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			for(int i = 0; i < SoftwareCfgGenerator::LM_ETHERNET_ADAPTERS_COUNT; i++)
			{
				SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties lmNetProperties;

				int ethernetAdapterNo = SoftwareCfgGenerator::LM_ETHERNET_ADAPTER1 + i;

				lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, ethernetAdapterNo, m_log);


				switch(ethernetAdapterNo)
				{
				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER1:
					// tuning data adapter
					//
					if (lmNetProperties.tuningEnable == true)
					{
						lmNetProperties.tuningIP = lmNetProperties.tuningIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.tuningIP) == true)
						{
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.tuningIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.tuningIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.tuningIP, lm);
						}
					}
					break;

				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER2:
				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER3:
					// appllication and diagnostics data adapters
					//
					if (lmNetProperties.appDataEnable == true)
					{
						lmNetProperties.appDataIP = lmNetProperties.appDataIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.appDataIP) == true)
						{
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.appDataIP];

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
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.diagDataIP];

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


	bool ApplicationLogicCompiler::disposeOptoModulesTxRxBuffers()
	{
		if (m_connections->count() == 0)
		{
			return true;
		}

		if (m_optoModuleStorage == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Dispose opto modules tx/rx buffers...")));

		result = m_optoModuleStorage->calculatePortsAbsoulteTxStartAddresses();

		if (result == false)
		{
			return false;
		}

		result = m_optoModuleStorage->setPortsRxDataSizes();

		if (result == false)
		{
			return false;
		}

		result = m_optoModuleStorage->calculatePortsRxStartAddresses();

		if (result == false)
		{
			return false;
		}

		LOG_SUCCESS(m_log, QString(tr("Ok")));

		return result;
	}

	// find all logic modules (LMs) in project
	// fills m_lm vector
	//
	void ApplicationLogicCompiler::findLMs()
	{
		m_lm.clear();

		findLM(m_deviceRoot);

		if (m_lm.count() == 0)
		{
			LOG_MESSAGE(m_log, tr("Logic modules (LMs) not found!"));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Found logic modules (LMs): %1")).arg(m_lm.count()));
		}
	}


	// find logic modules (LMs), recursive
	//
	void ApplicationLogicCompiler::findLM(Hardware::DeviceObject* startFromDevice)
	{
		if (startFromDevice == nullptr)
		{
			assert(startFromDevice != nullptr);

			msg = QString(tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__);

			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Signal)
		{
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Module)
		{
			Hardware::DeviceModule* module = reinterpret_cast<Hardware::DeviceModule*>(startFromDevice);

			if (module->moduleFamily() == Hardware::DeviceModule::FamilyType::LM)
			{
				Hardware::DeviceObject* parent = startFromDevice->parent();

				if (parent != nullptr)
				{
					if (parent->deviceType() == Hardware::DeviceType::Chassis)
					{
						// LM must be installed in the chassis
						//
						m_lm.append(reinterpret_cast<Hardware::DeviceModule*>(startFromDevice));
					}
					else
					{
						msg = QString(tr("LM %1 is not installed in the chassis")).arg(module->equipmentIdTemplate());

						LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, msg);

						qDebug() << msg;
					}
				}
			}

			return;
		}

		int childrenCount = startFromDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = startFromDevice->child(i);

			findLM(device);
		}
	}


	bool ApplicationLogicCompiler::compileModulesLogicsPass1()
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #1...")));

		// first compiler pass
		//
		for(int i = 0; i < m_lm.count(); i++)
		{
			ModuleLogicCompiler* moduleLogicCompiler = new ModuleLogicCompiler(*this, m_lm[i]);

			m_moduleCompilers.append(moduleLogicCompiler);

			result &= moduleLogicCompiler->firstPass();
		}

		return result;
	}


	bool ApplicationLogicCompiler::writeOptoConnectionsReport()
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

			if (cn->mode() == Hardware::OptoPort::Mode::Optical)
			{
				list.append(delim);
				str = QString(tr("Opto connection %1")).arg(cn->connectionID());
				list.append(str);
				list.append(delim);
				list.append("");

				Hardware::OptoPort* p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

				writeOptoPortInfo(p1, list);

				Hardware::OptoPort* p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

				writeOptoPortInfo(p2, list);
			}
			else
			{
				list.append(delim);
				str = QString(tr("RS232/485 connection %1")).arg(cn->connectionID());
				list.append(str);
				list.append(delim);
				list.append("");
			}
		}

		m_resultWriter->addFile("Reports", "opto-connections.txt", "", "", list);

		return true;
	}


	bool ApplicationLogicCompiler::writeOptoModulesReport()
	{
		QVector<Hardware::OptoModule*> modules = m_optoModuleStorage->getOptoModulesSorted();

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
			Hardware::OptoModule* module = modules[i];

			if (module == nullptr)
			{
				assert(false);
				continue;
			}

			list.append(delim);

			if (module->isLM())
			{
				str = QString(tr("Opto module LM %1")).arg(module->equipmentID());
			}
			else
			{
				if (module->isOCM())
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
			QVector<Hardware::OptoPort*> ports = module->getPortsSorted();

			for(Hardware::OptoPort* port : ports)
			{
				writeOptoPortInfo(port, list);
			}
		}

		m_resultWriter->addFile("Reports", "opto-modules.txt", "", "", list);

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

			if (cn->mode() == Hardware::OptoPort::Mode::Optical)
			{
				Hardware::OptoPort* p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
				Hardware::OptoPort* p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

				writeOptoVhdFile(cn->connectionID(), p1, p2);
				writeOptoVhdFile(cn->connectionID(), p2, p1);
			}
			else
			{
//				Hardware::OptoPort* p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
//				writeOptoVhdFile(cn->connectionID(), p1);
			}
		}

		return true;
	}


	bool ApplicationLogicCompiler::writeOptoVhdFile(const QString& connectionID, Hardware::OptoPort* outPort, Hardware::OptoPort* inPort)
	{
		if (outPort == nullptr || inPort == nullptr)
		{
			assert(false);
			return false;
		}

		if (outPort->txAnalogSignalsCount() + outPort->txDiscreteSignalsCount() == 0)
		{
			return true;
		}

		QString outPortID = outPort->equipmentID().toLower();
		QString inPortID = inPort->equipmentID().toLower();

		QString fileName = QString("%1.vhd").arg(inPortID);
		QStringList list;
		QString str;

		int inBusWidth = outPort->txDataSizeW() * sizeof(quint16) * 16;

		quint32 dataID = outPort->txDataID();

		QVector<Hardware::OptoPort::TxSignal> txAnalogs = outPort->txAnalogSignals();
		QVector<Hardware::OptoPort::TxSignal> txDiscretes = outPort->txDiscreteSignals();

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

		str = QString("-- Connection ID:\t%1").arg(connectionID);
		list.append(str);

		str = QString("-- Opto port ID:\t%1").arg(inPort->equipmentID());
		list.append(str);

		str = QString("-- Rx data size:\t%1 bytes").arg(outPort->txDataSizeW() * sizeof(quint16));
		list.append(str);

		str.sprintf("-- Rx data ID:\t\t%u (0x%08X)", dataID, dataID);
		list.append(str);

		list.append("--\n");

		// declaration section

		list.append("library ieee;");
		list.append("use ieee.std_logic_1164.all;");
		list.append("use ieee.numeric_std.all;\n");

		str = QString("entity %1 is port\n\t(\n").arg(inPortID);
		list.append(str);

		str = QString("\t\tconst_rx_data_id : out std_logic_vector(32-1 downto 0);");
		list.append(str);

		str = QString("\t\trx_data_id : out std_logic_vector(32-1 downto 0);\n");
		list.append(str);

		if (txAnalogs.count() > 0)
		{
			for(Hardware::OptoPort::TxSignal& txAnalog :  txAnalogs)
			{
				str = QString("\t\t%1 : out std_logic_vector(%2-1 downto 0);").
						arg(txAnalog.appSignalID.remove("#")).
						arg(txAnalog.sizeBit);

				list.append(str);
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::OptoPort::TxSignal& txDiscrete :  txDiscretes)
			{
				str = QString("\t\t%1 : out std_logic;").arg(txDiscrete.appSignalID.remove("#"));
				list.append(str);
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

		str = QString("begin").arg(inPortID).arg(inPortID);
		list.append(str);

		str = QString("\tin_data <= %1;\n").arg(outPortID);
		list.append(str);

		str = QString("\tconst_rx_data_id <= std_logic_vector(to_unsigned(%1,32));").arg(dataID);
		list.append(str);
		str = QString("\trx_data_id <= in_data(32-1 downto 0);\n").arg(dataID);
		list.append(str);

		if (txAnalogs.count() > 0)
		{
			for(Hardware::OptoPort::TxSignal& txAnalog :  txAnalogs)
			{
				str = QString("\t%1 <= in_data(%2-1 downto %3);").
						arg(txAnalog.appSignalID.remove("#")).
						arg(txAnalog.address.offset() * 16 + txAnalog.sizeBit).
						arg(txAnalog.address.offset() * 16);

				list.append(str);
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::OptoPort::TxSignal& txDiscrete :  txDiscretes)
			{
				str = QString("\t%1 <= in_data(%2);").
						arg(txDiscrete.appSignalID.remove("#")).
						arg(txDiscrete.address.offset() * 16 + txDiscrete.address.bit());
				list.append(str);
			}

			list.append("");
		}

		list.append("end arch;");

		m_resultWriter->addFile("Opto-vhd", fileName, list);

		return true;
	}


	void ApplicationLogicCompiler::writeOptoPortInfo(Hardware::OptoPort* port, QStringList& list)
	{
		if (port == nullptr)
		{
			assert(false);
			return;
		}

		QString str;

		str = QString(tr("Port equipmentID:\t%1")).arg(port->equipmentID());
		list.append(str);

		if (port->txDataSizeW() == 0)
		{
			str = QString(tr("Associated LM ID:\t%1")).arg(m_optoModuleStorage->getOptoPortAssociatedLmID(port));
			list.append(str);

			str = QString(tr("Port ID:\t\t-"));
			list.append(str);

			str = QString(tr("Linked port ID:\t\t%1")).arg(port->linkedPortID());
			list.append(str);

			str = QString(tr("Connection ID:\t\t%1\n\n")).arg(port->connectionID());
			list.append(str);

			return;
		}

		str = QString(tr("Associated LM ID:\t%1")).arg(m_optoModuleStorage->getOptoPortAssociatedLmID(port));
		list.append(str);

		str = QString(tr("Port ID:\t\t%1")).arg(port->portID());
		list.append(str);

		str = QString(tr("Linked port ID:\t\t%1")).arg(port->linkedPortID());
		list.append(str);

		str = QString(tr("Connection ID:\t\t%1")).arg(port->connectionID());
		list.append(str);

		str = QString(tr("TxData start address:\t%1")).arg(port->txStartAddress());
		list.append(str);

		str = QString(tr("TxData size:\t\t%1\n")).arg(port->txDataSizeW());
		list.append(str);

		list.append(QString(tr("Port txData:\n")));

		str.sprintf("%04d:%02d  [%04d:%02d]  TxDataID = 0x%08X (%u)", port->txStartAddress(), 0, 0, 0, port->txDataID(), port->txDataID());
		list.append(str);

		QVector<Hardware::OptoPort::TxSignal> txSignals = port->getTxSignals();

		for(const Hardware::OptoPort::TxSignal& tx : txSignals)
		{
			str.sprintf("%04d:%02d  [%04d:%02d]  %s",
						port->txStartAddress() + tx.address.offset(), tx.address.bit(),
						tx.address.offset(), tx.address.bit(),
						C_STR(tx.appSignalID));
			list.append(str);
		}

		list.append("");
		list.append("");
	}


	bool ApplicationLogicCompiler::compileModulesLogicsPass2()
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #2...")));

		// second compiler pass
		//
		for(int i = 0; i < m_moduleCompilers.count(); i++)
		{
			result &= m_moduleCompilers[i]->secondPass();
		}

		result &= m_resultWriter->writeMultichannelFiles();

		return result;
	}


	bool ApplicationLogicCompiler::writeBinCodeForLm(QString subsystemID, int subsystemKey, QString lmEquipmentID, QString lmCaption, int lmNumber, int frameSize, int frameCount, ApplicationLogicCode& appLogicCode)
	{
		if (m_resultWriter == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		bool result = true;

		QStringList metadataFields;

		appLogicCode.getAsmMetadataFields(metadataFields);

		MultichannelFile* multichannelFile = m_resultWriter->createMutichannelFile(subsystemID, subsystemKey, lmEquipmentID, lmCaption, frameSize, frameCount, metadataFields);

		if (multichannelFile != nullptr)
		{
			QByteArray binCode;

			appLogicCode.getBinCode(binCode);

			std::vector<QVariantList> metadata;

			appLogicCode.getAsmMetadata(metadata);

			result = multichannelFile->setChannelData(lmNumber, frameSize, frameCount, binCode, metadata);
		}
		else
		{
			result = false;
		}

		return result;
	}





}



