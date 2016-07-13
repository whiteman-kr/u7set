#include "ApplicationLogicCompiler.h"

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

			if (compileModulesLogicsPass1() == false) break;

			if (writeConnectionsFile() == false) break;

			if (disposeOptoModulesTxRxBuffers() == false) break;

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
				optoPort1->setEnable(connection->enable());
				optoPort1->setEnableDuplex(connection->enableDuplex());

				optoPort1->setManualSettings(connection->manualSettings());
				optoPort1->setManualTxSizeW(connection->port1ManualTxWordsQuantity());
				optoPort1->setManualRxSizeW(connection->port1ManualRxWordsQuantity());

				if (optoModule->deviceModule()->moduleFamily() == Hardware::DeviceModule::FamilyType::LM)
				{
					// LM's port '%1' can't work in RS232/485 mode (connection '%2').
					//
					m_log->errALC5020(connection->port1EquipmentID(), connection->connectionID());
					result = false;
				}

				optoPort1->addTxSignalsID(connection->signalList());

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

				if (m_optoModuleStorage->isCompatiblePorts(optoPort1, optoPort2) == false)
				{
					// Opto ports '%1' and '%2' are not compatible (connection '%3').
					//
					m_log->errALC5018(connection->port1EquipmentID(), connection->port2EquipmentID(), connection->connectionID());
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
				optoPort1->setManualTxSizeW(connection->port1ManualTxWordsQuantity());
				optoPort1->setManualRxSizeW(connection->port1ManualRxWordsQuantity());

				optoPort2->setPortID(portID);
				optoPort2->setMode(Hardware::OptoPort::Mode::Optical);
				optoPort2->setManualSettings(connection->manualSettings());
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

		result = m_optoModuleStorage->setPortsRxDataSizes();

		if (result == false)
		{
			return false;
		}

		result = m_optoModuleStorage->calculatePortsTxRxStartAddresses();

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


	bool ApplicationLogicCompiler::writeConnectionsFile()
	{
		QStringList list;

		int count = m_connections->count();

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
				str = QString("Opto connection %1").arg(cn->connectionID());
				list.append(str);
				list.append(delim);
				list.append("");

				str = QString("Port1 %1, txSignals:\n").arg(cn->port1EquipmentID());
				list.append(str);

				Hardware::OptoPort* p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

				if (p1 != nullptr)
				{
					list.append(p1->getTxSignalsID());
					list.append("");
				}

				str = QString("Port2 %1, txSignals:\n").arg(cn->port2EquipmentID());
				list.append(str);

				Hardware::OptoPort* p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

				if (p2 != nullptr)
				{
					list.append(p2->getTxSignalsID());
					list.append("");
				}
			}
			else
			{
				list.append(delim);
				str = QString(" RS232/485 connection %1").arg(cn->connectionID());
				list.append(str);
				list.append(delim);
				list.append("");

			}
		}

		m_resultWriter->addFile("Reports", "connections.txt", "", "", list);

		return true;
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


	bool ApplicationLogicCompiler::writeBinCodeForLm(QString subsysStrID, QString lmEquipmentID, QString lmCaption, int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode)
	{
		if (m_resultWriter == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		int subsysID = m_subsystems->ssKey(subsysStrID);

		if (subsysID == -1)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(lmCaption));

			return false;
		}

		bool result = true;

		MultichannelFile* multichannelFile = m_resultWriter->createMutichannelFile(subsysStrID, subsysID, lmEquipmentID, lmCaption, frameSize, frameCount);

		if (multichannelFile != nullptr)
		{
			result = multichannelFile->setChannelData(channel, frameSize, frameCount, appLogicBinCode);
		}
		else
		{
			result = false;
		}

		return result;
	}





}



