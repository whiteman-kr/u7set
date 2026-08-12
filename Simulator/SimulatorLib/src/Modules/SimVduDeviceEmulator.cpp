
#include "SimVduDeviceEmulator.h"

#include "../SimException.h"
#include "../SimulatorPrivate.h"


namespace Sim
{
	//
	// DeviceEmulator
	//
	VduDeviceEmulator::VduDeviceEmulator(SimulatorPrivate* simulator) :
		m_simulator{simulator},
		m_log{simulator->log(), "VduDeviceEmulator"}
	{
		return;
	}

	VduDeviceEmulator::~VduDeviceEmulator()
	{
		m_log.writeDebug("~VduDeviceEmulator");
		return;
	}

	bool VduDeviceEmulator::clear()
	{
		setLogicModuleInfo(Hardware::LogicModuleInfo());
		setDeviceState(DeviceState::Off);

		m_lmDescription.clear();
		m_logicModuleExtraInfo = {};

		m_confEeprom.clear();

		// m_lans.clear();

		// m_commands.clear();

		{
			m_cacheMutex.lock();
			m_cachedLogicModuleInfo = Hardware::LogicModuleInfo();
			m_cacheMutex.unlock();
		}

		return true;
	}

	DeviceError VduDeviceEmulator::init(const Hardware::LogicModuleInfo& logicModuleInfo, // Run from UI thread
										const LmDescription& lmDescription,
										const Eeprom& confEeprom,
										QString subsystemPath,
										const ConnectionsImpl& connections,
										const LogicModulesInfo& logicModulesExtraInfo)
	{
		clear();

		m_log.setOutputScope(QString("VduDeviceEmulator %1").arg(logicModuleInfo.equipmentId));
		m_log.writeDebug(tr("Init device."));

		// --
		//
		setLogicModuleInfo(logicModuleInfo);

		m_lmDescription = lmDescription;

		m_confEeprom = confEeprom;

		// Create specific CommnadProcessor
		//
		// m_commandProcessor = std::unique_ptr<CommandProcessor>(CommandProcessor::createInstance(this));

		// if (m_commandProcessor == nullptr)
		//{
		//	m_log.writeWarning(QString("There is no simulation for %1, LmDescription.name = %2")
		//						   .arg(logicModuleInfo.equipmentId)
		//						   .arg(lmDescription.name()));
		//	return DeviceError::NoCommandProcessor;
		// }

		// --
		//
		if (bool ok = initEeprom(); //
			ok == false)
		{
			return DeviceError::InitEepromError;
		}

		// Init RAM
		// Command processor needs initialized memory areas, so it can cache them to EmulatorCommand
		//
		initMemory();

		// Get LMs connections
		//
		m_connections = connections.lmConnections(equipmentId());

		// Set LogicModuleExtraInfo and get LAN Connections
		//
		if (std::optional<::LogicModuleInfo> extraInfo = logicModulesExtraInfo.get(equipmentId()); extraInfo.has_value() == false)
		{
			setLogicModuleExtraInfo({});

			m_log.writeError(tr("Information for LogicModule %1 is not found (file %2/%3)")
								 .arg(equipmentId())
								 .arg(Directory::COMMON)
								 .arg(File::LOGIC_MODULES_XML));

			return DeviceError::ModuleExtraInfoNotFound;
		}
		else
		{
			setLogicModuleExtraInfo(extraInfo.value());
		}

		// Init Module LAN  Intefaces
		//
		// if (bool ok = m_lans.init(logicModuleExtraInfo()); ok == false)
		//{
		//	return DeviceError::LanControllerError;
		//}

		// --
		//
		// if (bool ok = parseAppLogicCode(); ok == false)
		//{
		//	return DeviceError::ParsingAppLogicError;
		//}

		return DeviceError::Ok;
	}

	bool VduDeviceEmulator::powerOff()
	{
		m_log.writeDebug(tr("Off"));
		setDeviceState(DeviceState::Off);
		return true;
	}

	bool VduDeviceEmulator::reset()
	{
		assert(false); // TODO
		// m_log.writeDebug(tr("Reset"));
		// setDeviceState(DeviceState::Start);
		return true;
	}

	bool VduDeviceEmulator::runWorkcycle(std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle)
	{
		assert(false);
		return false;

		// bool ok = false;

		// if (deviceState() == DeviceState::Start)
		//{
		//	bool processStartOk = processStartMode();
		//	if (processStartOk == false)
		//	{
		//		return false;
		//	}
		// }

		// if (auto currentDeviceState = deviceState(); //
		//  currentDeviceState == DeviceState::Fault)
		//{
		//	ok = processFaultMode();
		// }
		// else
		//{
		//	if (currentDeviceState == DeviceState::Off)
		//	{
		//		ok = processOffMode();
		//	}
		//	else
		//	{
		//		Q_ASSERT(currentDeviceState == DeviceState::Operate);

		//		ok = processOperate(currentTime, currentDateTime, workcycle);
		//	}
		//}

		//// Perform post run cycle actions
		////
		// auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime);

		//// qDebug() << "DeviceEmulator::runWorkcycle " << ms.count() << ", Diff: " << QDateTime::currentDateTime().toMSecsSinceEpoch() -
		//// ms.count();

		// TimeStamp plantTime{ms.count() + currentDateTime.offsetFromUtc() * 1000};
		// TimeStamp localTime{plantTime};
		// TimeStamp systemTime{ms.count()};

		//// Set LogicModule's RAM to Sim::AppSignalManager
		////
		// m_simulator->appSignalManager().setData(equipmentId(), ram(), plantTime, localTime, systemTime);

		//// Send reg data to AppDataSrv
		////
		// if (deviceState() == DeviceState::Operate && m_lans.isAppDataEnabled() == true)
		//{
		//	QByteArray regData;

		//	quint32 regDataOffsetW = m_lmDescription.memory().m_appLogicWordDataOffset;
		//	quint32 regDataSizeW = m_logicModuleExtraInfo.appDataSizeBytes / 2;

		//	if (regDataSizeW > m_lmDescription.memory().m_appLogicWordDataSize)
		//	{
		//		m_log.writeError(tr("Send reg data error, buffer size is too big. LM %1, OffsetW %2, SizeW %3, AppLogicWordDataSize %4.")
		//							 .arg(equipmentId())
		//							 .arg(regDataOffsetW)
		//							 .arg(regDataSizeW)
		//							 .arg(m_lmDescription.memory().m_appLogicWordDataSize));

		//		setDeviceState(DeviceState::Fault);
		//		return false;
		//	}

		//	bool readRegBufferOk = m_ram.readToBuffer(regDataOffsetW, E::LogicModuleRamAccess::Write, regDataSizeW, &regData, true);
		//	if (readRegBufferOk == false)
		//	{
		//		m_log.writeError(tr("Error reading regBuffer for LM %1, OffsetW %2, SizeW %3.")
		//							 .arg(equipmentId())
		//							 .arg(regDataOffsetW)
		//							 .arg(regDataSizeW));

		//		setDeviceState(DeviceState::Fault);
		//		return false;
		//	}

		//	m_lans.sendAppData(regData, plantTime);
		//}

		// return ok;
	}

	// Sim::CyclePhase VduDeviceEmulator::phase() const
	//{
	//	return m_logicUnit.phase;
	// }

	// void VduDeviceEmulator::setPhase(Sim::CyclePhase value)
	//{
	//	m_logicUnit.phase = value;
	// }

	// quint32 VduDeviceEmulator::programCounter() const
	//{
	//	return m_logicUnit.programCounter;
	// }

	// void VduDeviceEmulator::setProgramCounter(quint32 value)
	//{
	//	m_logicUnit.programCounter = value;
	// }

	// quint32 VduDeviceEmulator::flagCmp() const
	//{
	//	return m_logicUnit.flags.cmp;
	// }

	// void VduDeviceEmulator::setFlagCmp(quint32 value)
	//{
	//	m_logicUnit.flags.cmp = value;
	// }

	// quint16 VduDeviceEmulator::acc() const
	//{
	//	return m_logicUnit.acc;
	// }

	// void VduDeviceEmulator::setAcc(quint16 value)
	//{
	//	m_logicUnit.acc = value;
	// }

	bool VduDeviceEmulator::initMemory()
	{
		assert(false);
		return false;

		// bool ok = true;

		// m_ram.reset();

		//// RAM - I/O modules
		////
		// const LmDescription::Memory& memory = m_lmDescription.memory();

		// for (quint32 i = 0; i < memory.m_moduleCount; i++)
		//{
		//	ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
		//							  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
		//							  memory.m_moduleDataSize,
		//							  false,
		//							  QString("Input I/O Module %1").arg(i + 1));

		//	ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
		//							  memory.m_moduleDataOffset + memory.m_moduleDataSize * i,
		//							  memory.m_moduleDataSize,
		//							  true,
		//							  QString("Output I/O Module %1").arg(i + 1));
		//}

		//// RAM - TX/RX Opto Interfaces
		////
		// const LmDescription::OptoInterface& optoInterface = m_lmDescription.optoInterface();

		// for (quint32 i = 0; i < optoInterface.m_optoPortCount; i++)
		//{
		//	ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
		//							  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
		//							  optoInterface.m_optoPortDataSize,
		//							  false,
		//							  QString("Rx Opto Port  %1").arg(i + 1));

		//	ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
		//							  optoInterface.m_optoInterfaceDataOffset + optoInterface.m_optoPortDataSize * i,
		//							  optoInterface.m_optoPortDataSize,
		//							  false,
		//							  QString("Tx Opto Port  %1").arg(i + 1));
		//}

		//// RAM - ApplicationLogic Block, bit/word access
		////
		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
		//						  memory.m_appLogicBitDataOffset,
		//						  memory.m_appLogicBitDataSize,
		//						  false,
		//						  QLatin1String("Application Logic Block (bit access)"));

		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::ReadWrite,
		//						  memory.m_appLogicWordDataOffset,
		//						  memory.m_appLogicWordDataSize,
		//						  false,
		//						  QLatin1String("Application Logic Block (word access)"));

		//// RAM - Tuninng Block
		////
		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
		//						  memory.m_tuningDataOffset,
		//						  memory.m_tuningDataSize,
		//						  false,
		//						  QLatin1String("Tuning Block"));

		//// Copy EEPROM tuning data to memory
		////
		//{
		//	Q_ASSERT(m_plainTuningData.size() % 2 == 0);

		//	const quint16* dataPtr = reinterpret_cast<const quint16*>(m_plainTuningData.constData());
		//	quint32 tuningDataOffset = lmDescription().memory().m_tuningDataOffset;

		//	for (int offsetW = 0; offsetW < m_plainTuningData.size() / 2; offsetW++)
		//	{
		//		quint16 data = dataPtr[offsetW];
		//		m_ram.writeWord(tuningDataOffset + offsetW, data, E::ByteOrder::NoEndian, E::LogicModuleRamAccess::Read);
		//	}
		//}

		//// RAM - Diag Data
		////
		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
		//						  memory.m_txDiagDataOffset,
		//						  memory.m_txDiagDataSize,
		//						  false,
		//						  QLatin1String("Input Diag Data"));

		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
		//						  memory.m_txDiagDataOffset,
		//						  memory.m_txDiagDataSize,
		//						  false,
		//						  QLatin1String("Output Diag Data"));


		//// RAM - App Data
		////
		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Read,
		//						  memory.m_appDataOffset,
		//						  memory.m_appDataSize,
		//						  false,
		//						  QLatin1String("Input App Data"));

		// ok &= m_ram.addMemoryArea(E::LogicModuleRamAccess::Write,
		//						  memory.m_appDataOffset,
		//						  memory.m_appDataSize,
		//						  false,
		//						  QLatin1String("Output App Data"));

		// Init memory for validity signals (set to 1).
		//
		// ok &= initValiditySignals();

		// return ok;
	}

	// Write 1 to all validity of input signals
	//
	// bool VduDeviceEmulator::initValiditySignals()
	//{
	//	bool result = true;
	//	const Sim::AppSignalManagerImpl& appSignalManager = m_simulator->appSignalManager();

	//	auto validityOffsets = appSignalManager.validityInputSignals(equipmentId());

	//	for (const auto& offset : validityOffsets)
	//	{
	//		bool ok = m_ram.writeBit(offset.address.offset(), offset.address.bit(), 1, E::ByteOrder::BigEndian, offset.ramAccess);

	//		if (ok == false)
	//		{
	//			m_log.writeError(QString("Error writing validity signal, offset %1, bit %2, RamAccess %2")
	//								 .arg(offset.address.offset())
	//								 .arg(offset.address.bit())
	//								 .arg(E::valueToString(offset.ramAccess)));
	//			result = false;
	//		}
	//	}

	//	return result;
	//}

	bool VduDeviceEmulator::initEeprom()
	{
		m_log.writeDebug(tr("Init EEPROM"));

		bool result = true;
		bool ok = true;

		ok = m_confEeprom.parseAllocationFrame(m_lmDescription.flashMemory().m_maxConfigurationCount);
		if (ok == false)
		{
			m_log.writeError(tr("Parse configuration EEPROM allocation frame error."));
			result = false;
		}

		// if (m_tuningEeprom.subsystemKey() != m_confEeprom.subsystemKey() ||
		//	m_tuningEeprom.subsystemKey() != m_appLogicEeprom.subsystemKey())
		//{
		//	QString str = tr("EEPROMs have different subsystemKeys: \n"
		//					 "\ttuningEeprom.subsystemKey: %1\n"
		//					 "\tconfEeprom.subsystemKey: %2\n"
		//					 "\tappLogicEeprom.subsystemKey: %3")
		//					  .arg(m_tuningEeprom.subsystemKey())
		//					  .arg(m_confEeprom.subsystemKey())
		//					  .arg(m_appLogicEeprom.subsystemKey());
		//	m_log.writeError(str);
		//	result = false;
		// }

		// if (m_tuningEeprom.buildNo() != m_confEeprom.buildNo() || m_tuningEeprom.buildNo() != m_appLogicEeprom.buildNo())
		//{
		//	QString str = tr("EEPROMs have different buildNo: \n"
		//					 "\ttuningEeprom.buildNo: %1\n"
		//					 "\tconfEeprom.buildNo: %2\n"
		//					 "\tappLogicEeprom.buildNo: %3")
		//					  .arg(m_tuningEeprom.buildNo())
		//					  .arg(m_confEeprom.buildNo())
		//					  .arg(m_appLogicEeprom.buildNo());
		//	m_log.writeError(str);
		//	result = false;
		// }

		// if (m_tuningEeprom.configurationCount() != m_confEeprom.configurationCount() ||
		//	m_tuningEeprom.configurationCount() != m_appLogicEeprom.configurationCount())
		//{
		//	QString str = tr("EEPROMs EEPROMs have different configurationCount: \n"
		//					 "\ttuningEeprom.configurationCount: %1\n"
		//					 "\tconfEeprom.configurationCount: %2\n"
		//					 "\tappLogicEeprom.configurationCount: %3")
		//					  .arg(m_tuningEeprom.configurationCount())
		//					  .arg(m_confEeprom.configurationCount())
		//					  .arg(m_appLogicEeprom.configurationCount());
		//	m_log.writeError(str);
		//	result = false;
		// }

		if (result == false)
		{
			return false;
		}

		return result;
	}

#if 0
	bool VduDeviceEmulator::processOperate(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 /*workcycle*/)
	{
		// qDebug() << "DeviceEmulator::processOperate " << equipmentId();

		// One LogicModule Cycle
		//

		// Initialization before work cycle
		//
		m_logicUnit = LogicUnitData{};

		bool result = m_commandProcessor->updatePlatformInterfaceState(currentDateTime);
		if (result == false)
		{
			return false;
		}

		// --
		//
		m_simulator->overrideSignals().updateRamOverrideData(equipmentId(), m_ram);

		// COMMENTED as for now there is no need to zero IO modules memory
		// as there is no control of reading uninitialized memory.
		//
		// m_ram.clearMemoryAreasOnStartCycle();				// Reset to 0 some memory areas before start work cycle (like memory area
		// for write i/o modules)

		// Get data from fiber optic channels (LM, OCM)
		// !!! receiveConnectionsData !!! was moved to Sim::Control,
		// as it is must be called before ALL LMs started to avoid gaps in communication
		//
		//		result = receiveConnectionsData(currentTime);
		//		if (result == false)
		//		{
		//			return false;
		//		}

		// Run work cycle
		//
		while (m_logicUnit.programCounter < m_plainAppLogic.size() &&
			   (m_logicUnit.phase == CyclePhase::IdrPhase || m_logicUnit.phase == CyclePhase::AlpPhase))
		{
			if (m_logicUnit.programCounter >= static_cast<int>(m_offsetToCommand.size()))
			{
				Q_ASSERT(false);
				SIM_FAULT("Command not found in current ProgramCounter.");
				break;
			}

			int commandIndex = m_offsetToCommand[m_logicUnit.programCounter];

			if (commandIndex == -1 || commandIndex > static_cast<int>(m_commands.size()))
			{
				SIM_FAULT(QString("Command not found for ProgramCounter %1").arg(m_logicUnit.programCounter));
				break;
			}

			EmulatorCommand& command = m_commands[commandIndex];
			Q_ASSERT(m_logicUnit.programCounter == command.m_offset);

			if (bool ok = runCommand(command); //
				ok == false && deviceState() != DeviceState::Fault)
			{
				SIM_FAULT(QString("Run command %1 unknown error.").arg(command.m_string));
				result = false;
				break;
			}

			if (deviceState() == DeviceState::Fault)
			{
				result = true;
				break;
			}

			// If ProgramCounter was not changed in runCommand (can be changed by APPSTART command), then
			// increment ProgramCounter to command size
			//
			if (m_logicUnit.programCounter == command.m_offset)
			{
				m_logicUnit.programCounter += command.m_size;
			}
		}

		// Send data to fiber optic channels (LM, OCM)
		//
		result &= sendConnectionsData(currentTime);

		return result;
	}
#endif

	void VduDeviceEmulator::fault(QString reasone, QString func)
	{
		// QString phase;
		// switch (m_logicUnit.phase)
		//{
		// case CyclePhase::IdrPhase:
		//	phase = "IDR";
		//	break;
		// case CyclePhase::AlpPhase:
		//	phase = "ALP";
		//	break;
		// case CyclePhase::ODT:
		//	phase = "ODT";
		//	break;
		// case CyclePhase::ST:
		//	phase = "ST";
		//	break;
		// default:
		//	Q_ASSERT(false);
		// }

		// QString str1 = QString("Fault");
		// QString str2 = QString("\tPhase: %1, ProgramCounter: %2 (0x%3), function: %4")
		//				   .arg(phase)
		//				   .arg(m_logicUnit.programCounter)
		//				   .arg(m_logicUnit.programCounter, 4, 16, QChar('0'))
		//				   .arg(func);
		// QString str3 = QString("\tReason: %1").arg(reasone);

		// m_log.writeError(str1);
		// m_log.writeError(str2);
		// m_log.writeError(str3);

		// setDeviceState(DeviceState::Fault);

		return;
	}

	// bool VduDeviceEmulator::processOffMode()
	//{
	//	Q_ASSERT(deviceState() == DeviceState::Off);
	//	return true;
	// }

	// bool VduDeviceEmulator::processStartMode()
	//{
	//	Q_ASSERT(deviceState() == DeviceState::Start);
	//	m_log.writeDebug(tr("Start mode"));

	//	bool ok = initMemory();
	//	if (ok == false)
	//	{
	//		m_log.writeError(tr("Init memory error."));
	//		setDeviceState(DeviceState::Fault);
	//		return false;
	//	}

	//	m_afbComponents.resetState(); // It will clear all AFBs' params

	//	setDeviceState(DeviceState::Operate);
	//	setRuntimeMode(RuntimeMode::RunMode);

	//	return true;
	//}

	// bool VduDeviceEmulator::processFaultMode()
	//{
	//	// Relax, you can do nothing
	//	//
	//	return true;
	// }

#if 0
	bool VduDeviceEmulator::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		if (auto ds = deviceState(); ds == DeviceState::Off || ds == DeviceState::Fault)
		{
			return true;
		}

		for (ConnectionImplPtr& c : m_connections)
		{
			// if (c->enabled() == false)
			//{
			//  Even though the connection is disabled we should procced it to zero
			//  memory and to set validity flag. If connection is disabled nothing will be send and
			//  then nothing can be received, it will cause timeout
			//
			//}

			// Get port for this connection for this LM
			//
			ConnectionPort* port = c->portForLmRawPtr(equipmentIdHash());
			if (port == nullptr)
			{
				Q_ASSERT(port);
				SIM_FAULT(QString("Communication port not found for connection %1 in LM %2.").arg(c->connectionId()).arg(equipmentId()));
				return false;
			}

			// Get receive buffer for port
			//
			const ::ConnectionPortInfo& portInfo = port->portInfo();

			Q_ASSERT(portInfo.lmID == equipmentId());

			// std::vector<char> rb;
			// std::vector<char>* receiveBuffer = &rb;
			std::vector<char>* receiveBuffer = c->getPortReceiveBuffer(portInfo.portNo);
			if (receiveBuffer == nullptr)
			{
				SIM_FAULT(QString("Get port receive buffer error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			// Get data (actually swap) to receive buffer
			//
			bool timeout = false;
			bool ok = c->receiveData(portInfo.portNo,
									 receiveBuffer,
									 currentTime,
									 std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration * 2}, // timeout
									 &timeout);

			if (ok == false)
			{
				SIM_FAULT(QString("Receive data error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			if (timeout == true)
			{
				// If receive buffer is empty then it is timeout
				//
				// NO: Clear memory in dedicated memory area
				//	   Don't do it:	m_ram.clearMemoryArea(portInfo.rxBufferAbsAddr, E::LogicModuleRamAccess::Read);
				//
				// YES: Cannot clear whole memory area, because it can be applied only for LM,
				//		where each opto port has its own memery area
				//		BUT OCM has shared memory area for several connections,
				//		thats why we need to clear just some data.
				//		Use: m_ram.setMem(...)
				//
				m_ram.setMem(portInfo.rxBufferAbsAddr, portInfo.rxDataSizeW, 0, E::LogicModuleRamAccess::Read);
			}
			else
			{
				if (receiveBuffer->empty() == true)
				{
					// If timeout not happened yet, but receiveBuffer is empty, wait mor time
					// do not exit from function here, later validity bit will be written
					//
				}
				else
				{
					// Payload is received
					// Write data to memory
					//
					Q_ASSERT(receiveBuffer->size() % 2 == 0);

					if (receiveBuffer->size() / 2 != portInfo.rxDataSizeW)
					{
						SIM_FAULT(QString("Receive data error, expected %1 words but received %2 words, connection %3, port %4 (%5).")
									  .arg(portInfo.rxDataSizeW)
									  .arg(receiveBuffer->size() / 2)
									  .arg(c->connectionId())
									  .arg(portInfo.portNo)
									  .arg(portInfo.equipmentID));
						return false;
					}

					ok = m_ram.writeBuffer(portInfo.rxBufferAbsAddr, E::LogicModuleRamAccess::Read, *receiveBuffer);
					if (ok == false)
					{
						SIM_FAULT(QString("Received buffer write memory error, %1 words, connection %2, port %3 (%4).")
									  .arg(portInfo.rxDataSizeW)
									  .arg(c->connectionId())
									  .arg(portInfo.portNo)
									  .arg(portInfo.equipmentID));
						return false;
					}

					// Check the DataID which is the part of the payload (first 4 bytes)
					//
					if (c->connectionInfo().disableDataIDControl == false)
					{
						quint32 receivedDataId = readRamDword(portInfo.rxBufferAbsAddr);

						if (port->portInfo().rxDataID != receivedDataId)
						{
							SIM_FAULT(QString("Received DataID mismatch, received 0x%1, expected 0x%2, connection %3, port %4 (%5).")
										  .arg(receivedDataId, 8, 16, QChar('0'))
										  .arg(port->portInfo().rxDataID, 8, 16, QChar('0'))
										  .arg(c->connectionId())
										  .arg(portInfo.portNo)
										  .arg(portInfo.equipmentID));
							return false;
						}
					}
				}
			}

			// Set port receive validity flag to 0 or 1
			//

			// Commented: For perfomamce reason it was moved to port, where memory area can be cached.
			//
			//			ok = m_ram.writeBit(portInfo.rxValiditySignalAbsAddr.offset(),
			//								static_cast<quint16>(portInfo.rxValiditySignalAbsAddr.bit()),
			//								timeout ? 0x0000 : 0x0001,
			//								E::ByteOrder::BigEndian,
			//								E::LogicModuleRamAccess::Read);

			ok = port->writeValidityBit(m_ram, timeout ? 0x0000 : 0x0001);

			if (ok == false)
			{
				SIM_FAULT(QString("Write receive validity signal error, signal %1 (%2), connection %3, port %4 (%5).")
							  .arg(portInfo.rxValiditySignalEquipmentID)
							  .arg(portInfo.rxValiditySignalAbsAddr.toString())
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}
		}

		return true;
	}
#endif

#if 0
	bool VduDeviceEmulator::sendConnectionsData(std::chrono::microseconds currentTime)
	{
		if (auto ds = deviceState(); //
			ds == DeviceState::Off || ds == DeviceState::Fault)
		{
			return true;
		}

		for (ConnectionImplPtr& c : m_connections)
		{
			if (c->enabled() == false)
			{
				// Connection is disabled, just skip it
				//
				continue;
			}

			// Get port for this connection for this LM
			//
			const ConnectionPort* port = c->portForLmRawPtr(equipmentIdHash());
			if (port == nullptr)
			{
				Q_ASSERT(port);
				SIM_FAULT(QString("Communication port not found for connection %1 in LM %2.").arg(c->connectionId()).arg(equipmentId()));
				return false;
			}

			// Get send buffer for port
			//
			const ::ConnectionPortInfo& portInfo = port->portInfo();

			Q_ASSERT(portInfo.lmID == equipmentId());

			// std::vector<char> sb;
			// std::vector<char>* sendBuffer = &sb;
			std::vector<char>* sendBuffer = c->getPortSendBuffer(portInfo.portNo);
			if (sendBuffer == nullptr)
			{
				SIM_FAULT(QString("Get port send buffer error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			// Write data from RAM to send buffer
			//
			bool ok = m_ram.readToBuffer(portInfo.txBufferAbsAddr, E::LogicModuleRamAccess::Write, portInfo.txDataSizeW, sendBuffer);
			if (ok == false)
			{
				SIM_FAULT(
					QString("Send data error, read data from memory (address %1, count %2) returned error, connection %3, port %4 (%5).")
						.arg(portInfo.txBufferAbsAddr)
						.arg(portInfo.txDataSizeW)
						.arg(c->connectionId())
						.arg(portInfo.portNo)
						.arg(portInfo.equipmentID));
				return false;
			}

			// Send data (actually swap) to send buffer
			//
			ok = c->sendData(portInfo.portNo, sendBuffer, currentTime);

			if (ok == false)
			{
				SIM_FAULT(QString("Send data error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}
		}

		return true;
	}
#endif


	//// Getting data from m_plainAppLogic
	////
	// template<typename TYPE>
	// TYPE VduDeviceEmulator::getData(int eepromOffset) const
	//{
	//	// eepromOffset - in bytes
	//	//
	//	if (eepromOffset < 0 || eepromOffset > static_cast<int>(m_plainAppLogic.size() - sizeof(TYPE)))
	//	{
	//		Q_ASSERT(eepromOffset >= 0 && static_cast<int>(eepromOffset - sizeof(TYPE)) <= m_plainAppLogic.size());
	//		return 0;
	//	}

	//	TYPE result = qFromBigEndian<TYPE>(m_plainAppLogic.constData() + eepromOffset);
	//	return result;
	//}

	ScopedLog& VduDeviceEmulator::log()
	{
		return m_log;
	}

	const QString& VduDeviceEmulator::equipmentId() const
	{
		return m_logicModuleInfo.equipmentId;
	}

	Hash VduDeviceEmulator::equipmentIdHash() const
	{
		return m_logicModuleIdHash;
	}

	int VduDeviceEmulator::buildNo() const
	{
		// On loading eeprom buildNo was checked, so it is guarantee to be the same across all the eeproms
		//
		return m_confEeprom.buildNo();
	}

	Hardware::LogicModuleInfo VduDeviceEmulator::logicModuleInfo() const
	{
		m_cacheMutex.lock();
		Hardware::LogicModuleInfo result = m_cachedLogicModuleInfo;
		m_cacheMutex.unlock();

		return result;
	}

	void VduDeviceEmulator::setLogicModuleInfo(const Hardware::LogicModuleInfo& lmInfo)
	{
		m_logicModuleInfo = lmInfo;
		m_logicModuleIdHash = calcHash(m_logicModuleInfo.equipmentId);

		m_cacheMutex.lock();
		m_cachedLogicModuleInfo = lmInfo;
		m_cacheMutex.unlock();

		return;
	}

	const ::LogicModuleInfo& VduDeviceEmulator::logicModuleExtraInfo() const
	{
		return m_logicModuleExtraInfo;
	}

	void VduDeviceEmulator::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_logicModuleExtraInfo = value;
	}

	const LmDescription& VduDeviceEmulator::lmDescription() const
	{
		return m_lmDescription;
	}

	// const Ram& VduDeviceEmulator::ram() const
	//{
	//	return m_ram;
	// }

	// Ram& VduDeviceEmulator::mutableRam()
	//{
	//	return m_ram;
	// }

	// const Lans& VduDeviceEmulator::lans() const
	//{
	//	return m_lans;
	// }

	// RuntimeMode VduDeviceEmulator::runtimeMode() const
	//{
	//	return m_runtimeMode.load();
	// }

	// void VduDeviceEmulator::setRuntimeMode(RuntimeMode value)
	//{
	//	RuntimeMode currentRuntimeMode = m_runtimeMode.exchange(value);

	//	if (currentRuntimeMode == RuntimeMode::TuningMode && value != RuntimeMode::TuningMode)
	//	{
	//		// Module leaves TuningMode, LAN must be notified
	//		//
	//		m_lans.tuningModeLeft();
	//	}

	//	return;
	//}

	// DeviceState VduDeviceEmulator::deviceState() const
	//{
	//	return m_deviceState2.load();
	// }

	void VduDeviceEmulator::setDeviceState(DeviceState value)
	{
		m_deviceState2.store(value);

		//	switch (value)
		//	{
		//	case DeviceState::Fault:
		//		setRuntimeMode(RuntimeMode::FaultedMode);
		//		break;
		//	case DeviceState::Off:
		//		setRuntimeMode(RuntimeMode::PoweredOffMode);
		//		break;
		//	case DeviceState::Start:
		//		setRuntimeMode(RuntimeMode::StartupMode);
		//		break;
		//	default:
		//		break;
		//	}

		return;
	}

	// bool VduDeviceEmulator::armingKey() const
	//{
	//	return m_armingKey.load();
	// }

	// void VduDeviceEmulator::setArmingKey(bool value)
	//{
	//	m_armingKey = value;
	// }

	// bool VduDeviceEmulator::tuningKey() const
	//{
	//	return m_tuningKey.load();
	// }

	// void VduDeviceEmulator::setTuningKey(bool value)
	//{
	//	m_tuningKey = value;
	// }

	// bool VduDeviceEmulator::sorIsSet() const
	//{
	//	return m_sorIsSet.load();
	// }

	// void VduDeviceEmulator::setSorIsSet(bool value)
	//{
	//	m_sorIsSet = value;
	// }

	// bool VduDeviceEmulator::sorSetSwitch1() const
	//{
	//	return m_sorSetSwitch1.load();
	// }

	// void VduDeviceEmulator::setSorSetSwitch1(bool value)
	//{
	//	m_sorSetSwitch1 = value;
	// }

	// bool VduDeviceEmulator::sorSetSwitch2() const
	//{
	//	return m_sorSetSwitch2.load();
	// }

	// void VduDeviceEmulator::setSorSetSwitch2(bool value)
	//{
	//	m_sorSetSwitch2 = value;
	// }

	// bool VduDeviceEmulator::sorSetSwitch3() const
	//{
	//	return m_sorSetSwitch3.load();
	// }

	// void VduDeviceEmulator::setSorSetSwitch3(bool value)
	//{
	//	m_sorSetSwitch3 = value;
	// }

	// bool VduDeviceEmulator::testSorResetSwitch(bool newValue)
	//{
	//	return m_sorResetSwitch.exchange(newValue);
	// }

} // namespace Sim
