
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

		{
			m_cacheMutex.lock();
			m_cachedLogicModuleInfo = Hardware::LogicModuleInfo();
			m_cacheMutex.unlock();
		}

		m_shared.memoryKey.clear();
		m_shared.clientSharedMemory.reset();

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

		// --
		//
		m_confEeprom = confEeprom;

		if (bool ok = initEeprom(); //
			ok == false)
		{
			return DeviceError::InitEepromError;
		}

		// Get LMs connections
		//
		m_connections = connections.lmConnections(equipmentId());

		// Set LogicModuleExtraInfo and get LAN Connections
		//
		if (std::optional<::LogicModuleInfo> extraInfo = logicModulesExtraInfo.get(equipmentId()); //
			extraInfo.has_value() == false)
		{
			// Now for VDU ::LogicModuleInfo is not stored in file Commin/LogicModules.xml, maybe it will be in future, so for now we just
			// log this error and continue.
			//
			::LogicModuleInfo ei{};
			ei.equipmentID = equipmentId();
			ei.caption = logicModuleInfo.equipmentId;
			ei.subsystemID = logicModuleInfo.subsystemId;
			ei.subsystemKey = 0;
			ei.lmNumber = logicModuleInfo.lmNumber;
			ei.subsystemChannel = QString{"%1"}.arg(QChar('A' + static_cast<int>(logicModuleInfo.channel)));

			setLogicModuleExtraInfo(ei);

			m_log.writeWarning(tr("Information for VDU %1 is not found (file %2/%3)")
								   .arg(equipmentId())
								   .arg(Directory::COMMON)
								   .arg(File::LOGIC_MODULES_XML));

			// Uncomment the next line when VDU ::LogicModuleInfo will be stored in file Commin/LogicModules.xml
			//
			// return DeviceError::ModuleExtraInfoNotFound;
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
		m_log.writeDebug(tr("Reset"));
		setDeviceState(DeviceState::Start);
		return true;
	}

	bool VduDeviceEmulator::runWorkcycle(std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle)
	{
		if (m_shared.clientSharedMemory.has_value() == false)
		{
			// Create shared memory for VDU simulator
			//
			m_shared.memoryKey = QString{"VduSimulator.pid:%1.eid:%2"}.arg(qApp->applicationPid()).arg(equipmentId());
			m_shared.clientSharedMemory.emplace(m_shared.memoryKey);

			bool created = m_shared.clientSharedMemory->create(sizeof(SimVduSharedData::SharedData), QSharedMemory::ReadWrite);
			if (created == false)
			{
				if (m_shared.clientSharedMemory->error() != QSharedMemory::AlreadyExists)
				{
					m_log.writeError(tr("Create shared memory error: %1").arg(m_shared.clientSharedMemory->errorString()));
					m_shared.clientSharedMemory.reset();

					setDeviceState(DeviceState::Fault);
					return false;
				}

				// If shared memory already exists, try to attach to it
				//
				bool attached = m_shared.clientSharedMemory->attach(QSharedMemory::ReadWrite);
				if (attached == false)
				{
					m_log.writeError(tr("Attach to existing shared memory error: %1").arg(m_shared.clientSharedMemory->errorString()));
					m_shared.clientSharedMemory.reset();
					setDeviceState(DeviceState::Fault);
					return false;
				}
			}
			else
			{
				// Just created shared memory, initialize it
				//
				assert(m_shared.clientSharedMemory->isAttached() == true);

				auto sharedData = static_cast<SimVduSharedData::SharedData*>(m_shared.clientSharedMemory->data());
				if (sharedData == nullptr)
				{
					m_log.writeError(tr("Shared memory data pointer is null."));
					m_shared.clientSharedMemory.reset();
					setDeviceState(DeviceState::Fault);
					return false;
				}

				std::scoped_lock lock{*m_shared.clientSharedMemory};
				*sharedData = SimVduSharedData::SharedData{};
				sharedData->size = sizeof(SimVduSharedData::SharedData);
			}
		}

		auto postCycleVduEmulatorUpdateFunc = [this, currentTime]()
		{
			if (m_shared.clientSharedMemory.has_value() == false)
			{
				m_log.writeError(tr("VduDeviceEmulator::runWorkcycle: Shared memory is not initialized."));
				setDeviceState(DeviceState::Fault);
				return;
			}

			if (m_shared.clientSharedMemory->isAttached() == false)
			{
				m_log.writeError(
					tr("VduDeviceEmulator::runWorkcycle: Shared memory attach error %1.").arg(m_shared.clientSharedMemory->errorString()));
				setDeviceState(DeviceState::Fault);
				return;
			}

			std::scoped_lock lock{*m_shared.clientSharedMemory};

			auto sharedData = static_cast<SimVduSharedData::SharedData*>(m_shared.clientSharedMemory->data());
			sharedData->simulatorState.updateCounter++;
			sharedData->simulatorState.deviceState = static_cast<int32_t>(deviceState());
			sharedData->simulatorState.plantTime = currentTime.count();

			return;
		};

		struct ScopeExit
		{
			std::function<void()> fn;
			~ScopeExit()
			{
				assert(fn);
				fn();
			}
		} guard{.fn = postCycleVduEmulatorUpdateFunc};

		if (deviceState() == DeviceState::Start)
		{
			bool processStartOk = processStartMode();
			if (processStartOk == false)
			{
				return false;
			}
		}

		bool ok = false;

		if (auto currentDeviceState = deviceState(); //
			currentDeviceState == DeviceState::Fault)
		{
			ok = processFaultMode();
		}
		else
		{
			if (currentDeviceState == DeviceState::Off)
			{
				ok = processOffMode();
			}
			else
			{
				Q_ASSERT(currentDeviceState == DeviceState::Operate);
				ok = processOperate(currentTime, currentDateTime, workcycle);
			}
		}

		return ok;
	}

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

		if (result == false)
		{
			return false;
		}

		return result;
	}

	bool VduDeviceEmulator::processOperate(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 /*workcycle*/)
	{
		// qDebug() << "DeviceEmulator::processOperate " << equipmentId();

		// Send data to fiber optic channels (LM, OCM, ...)
		//
		bool ok = sendConnectionsData(currentTime);
		return ok;
	}

	void VduDeviceEmulator::fault(QString reasone, QString func)
	{
		m_log.writeError(QString{"FAULT: %1, func: %2"}.arg(reasone).arg(func));
		setDeviceState(DeviceState::Fault);
		return;
	}

	bool VduDeviceEmulator::processOffMode()
	{
		Q_ASSERT(deviceState() == DeviceState::Off);
		return true;
	}

	bool VduDeviceEmulator::processStartMode()
	{
		Q_ASSERT(deviceState() == DeviceState::Start);
		m_log.writeDebug(tr("Start mode"));

		setDeviceState(DeviceState::Operate);
		setRuntimeMode(RuntimeMode::RunMode);

		return true;
	}

	bool VduDeviceEmulator::processFaultMode()
	{
		// Relax, you can do nothing
		//
		return true;
	}

	bool VduDeviceEmulator::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		if (auto ds = deviceState(); //
			ds == DeviceState::Off || ds == DeviceState::Fault)
		{
			return true;
		}

		auto receivedPortsData = std::make_unique<SimVduSharedData::OptoPorts>(SimVduSharedData::OptoPorts{});

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
				SIM_FAULT(QString("Communication port not found for connection %1 in VDU %2.").arg(c->connectionId()).arg(equipmentId()));
				return false;
			}

			// Get receive buffer for port
			//
			const ::ConnectionPortInfo& portInfo = port->portInfo();
			Q_ASSERT(portInfo.lmID == equipmentId());

			if (portInfo.portNo < 0 || portInfo.portNo > SimVduSharedData::OptoPortCount - 1)
			{
				SIM_FAULT(QString("Invalid port number %1 for connection %2 in VDU %3.")
							  .arg(portInfo.portNo)
							  .arg(c->connectionId())
							  .arg(equipmentId()));
				return false;
			}

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
			bool wasTimeout = false;

			bool ok = c->receiveData(portInfo.portNo,
									 receiveBuffer,
									 currentTime,
									 std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration * 2},
									 &wasTimeout);

			if (ok == false)
			{
				SIM_FAULT(QString("Receive data error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			if (wasTimeout == true)
			{
				// If receive buffer is empty then it is timeout
				// Clear memory in dedicated memory area, receivedPortsData is already filled with 0s.
				//
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
					Q_ASSERT_X(receiveBuffer->size() >= 4,
							   "VduDeviceEmulator::receiveConnectionsData",
							   "First 4 bytes is DataID, so we need at least 4 bytes to check it");

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

					if (receiveBuffer->size() > sizeof(SimVduSharedData::OptoBuffer::data))
					{
						SIM_FAULT(QString("Receive data error, received buffer size %1 is too big, connection %2, port %3 (%4).")
									  .arg(receiveBuffer->size())
									  .arg(c->connectionId())
									  .arg(portInfo.portNo)
									  .arg(portInfo.equipmentID));
						return false;
					}

					std::memcpy(receivedPortsData->portData[portInfo.portNo].data, receiveBuffer->data(), receiveBuffer->size());
					receivedPortsData->portData[portInfo.portNo].occupied = receiveBuffer->size();

					// Check the DataID which is the part of the payload (first 4 bytes).
					//
					if (c->connectionInfo().disableDataIDControl == false)
					{
						// Get uint32 in big endian on offest receiveBuffer->data()
						//
						const quint32 receivedDataId = qFromBigEndian<quint32>(receiveBuffer->data());

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
			receivedPortsData->portData[portInfo.portNo].timeout = wasTimeout ? 1 : 0;
		}

		// Write received data to shared memory
		//
		if (m_shared.clientSharedMemory.has_value() == false)
		{
			m_log.writeError(tr("VduDeviceEmulator::receiveConnectionsData: Shared memory is not initialized."));
			setDeviceState(DeviceState::Fault);
			return false;
		}

		if (m_shared.clientSharedMemory->isAttached() == false)
		{
			m_log.writeError(tr("VduDeviceEmulator::receiveConnectionsData: Shared memory attach error %1.")
								 .arg(m_shared.clientSharedMemory->errorString()));
			setDeviceState(DeviceState::Fault);
			return false;
		}

		{
			std::scoped_lock guar{*m_shared.clientSharedMemory};

			SimVduSharedData::SharedData* sharedData = static_cast<SimVduSharedData::SharedData*>(m_shared.clientSharedMemory->data());

			auto counter = sharedData->toVdu.updateCounter;
			sharedData->toVdu = *receivedPortsData;
			sharedData->toVdu.updateCounter = counter + 1;
		}

		return true;
	}

	bool VduDeviceEmulator::sendConnectionsData(std::chrono::microseconds currentTime)
	{
		if (auto ds = deviceState(); //
			ds == DeviceState::Off || ds == DeviceState::Fault)
		{
			return true;
		}

		// Fetch data from shared memory to send to connections
		//
		auto fromVduPortsData = std::make_unique<SimVduSharedData::OptoPorts>(SimVduSharedData::OptoPorts{});

		{
			// m_shared.lastOptoFromVduCounter

			if (m_shared.clientSharedMemory.has_value() == false)
			{
				m_log.writeError(tr("VduDeviceEmulator::sendConnectionsData: Shared memory is not initialized."));
				setDeviceState(DeviceState::Fault);
				return false;
			}

			if (m_shared.clientSharedMemory->isAttached() == false)
			{
				m_log.writeError(tr("VduDeviceEmulator::sendConnectionsData: Shared memory attach error %1.")
									 .arg(m_shared.clientSharedMemory->errorString()));
				setDeviceState(DeviceState::Fault);
				return false;
			}

			{
				std::scoped_lock guar{*m_shared.clientSharedMemory};

				auto sharedData = static_cast<SimVduSharedData::SharedData*>(m_shared.clientSharedMemory->data());
				*fromVduPortsData = sharedData->fromVdu;
			}
		}

		if (fromVduPortsData->updateCounter == m_shared.lastOptoFromVduCounter)
		{
			// No new data to send
			//
			return true;
		}
		else
		{
			// Update lastOptoFromVduCounter to the new value, so next time we will know if new data is available
			//
			m_shared.lastOptoFromVduCounter = fromVduPortsData->updateCounter;
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

			if (portInfo.portNo < 0 || portInfo.portNo > SimVduSharedData::OptoPortCount - 1)
			{
				SIM_FAULT(QString("Invalid port number %1 for connection %2 in LM %3.")
							  .arg(portInfo.portNo)
							  .arg(c->connectionId())
							  .arg(equipmentId()));
				return false;
			}

			if ((portInfo.txDataSizeW * 2) != fromVduPortsData->portData[portInfo.portNo].occupied)
			{
				SIM_FAULT(QString("Send data error, expected %1 bytes but received %2 bytes, connection %3, port %4 (%5).")
							  .arg(portInfo.txDataSizeW * 2)
							  .arg(fromVduPortsData->portData[portInfo.portNo].occupied)
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			//std::vector<char> sb;
			//std::vector<char>* sendBuffer = &sb;
			std::vector<char>* sendBuffer = c->getPortSendBuffer(portInfo.portNo);
			if (sendBuffer == nullptr)
			{
				SIM_FAULT(QString("Get port send buffer error, connection %1, port %2 (%3).")
							  .arg(c->connectionId())
							  .arg(portInfo.portNo)
							  .arg(portInfo.equipmentID));
				return false;
			}

			sendBuffer->resize(portInfo.txDataSizeW * 2);
			std::memcpy(sendBuffer->data(), fromVduPortsData->portData[portInfo.portNo].data, sendBuffer->size());

			// Send data (actually swap) to send buffer
			//
			if (bool sendOk = c->sendData(portInfo.portNo, sendBuffer, currentTime); //
				sendOk == false)
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

	// const Lans& VduDeviceEmulator::lans() const
	//{
	//	return m_lans;
	// }

	RuntimeMode VduDeviceEmulator::runtimeMode() const
	{
		return m_runtimeMode.load();
	}

	void VduDeviceEmulator::setRuntimeMode(RuntimeMode value)
	{
		m_runtimeMode.store(value);
		return;
	}

	DeviceState VduDeviceEmulator::deviceState() const
	{
		return m_deviceState2.load();
	}

	void VduDeviceEmulator::setDeviceState(DeviceState value)
	{
		m_deviceState2.store(value);

		switch (value)
		{
		case DeviceState::Fault:
			setRuntimeMode(RuntimeMode::FaultedMode);
			break;
		case DeviceState::Off:
			setRuntimeMode(RuntimeMode::PoweredOffMode);
			break;
		case DeviceState::Start:
			setRuntimeMode(RuntimeMode::StartupMode);
			break;
		default:
			break;
		}

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
} // namespace Sim
