#include <string.h>

#include "ModbusTcpSlaveGatewayHandler.h"
#include <Network.pb.h>

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::ModbusTcpSlaveHandler::SignalStyate struct implementation
	//
	// ---------------------------------------------------------------------------------

	ModbusTcpSlaveHandler::SignalState::SignalState(const ModbusFormat& frmt, const Address16& addr16)
	{
		format = frmt;
		modbusAddress = addr16;

		reverseBytes =	(std::endian::native == std::endian::little &&
						format.byteOrder == E::ModbusByteOrder::BE) ||
						(std::endian::native == std::endian::big &&
						format.byteOrder == E::ModbusByteOrder::LE);
	}

	// ---------------------------------------------------------------------------------
	//
	//	Gateway::ModbusTcpSlaveHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	ModbusTcpSlaveHandler::ModbusTcpSlaveHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 ModbusTcpSlaveGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(swInfo, settings, log, logGatewayPackets),
		m_softwareInfo(swInfo),
		m_appDataService1(settings.appDataService1.address),
		m_appDataService2(settings.appDataService2.address),
		m_gateway(gateway),
		m_appSignals(appSignals)
	{
	}

	ModbusTcpSlaveHandler::~ModbusTcpSlaveHandler()
	{
		shutdown();
	}

	void ModbusTcpSlaveHandler::run()
	{
		init();

		m_appDataServiceClientThread =
				new AppDataServiceClientThread( m_softwareInfo,
												m_appDataService1,
												m_appDataService2,
												QString("GatewayService %1").arg(m_softwareInfo.equipmentID()),
												this, m_log);
		m_appDataServiceClientThread->start();

		if (listeningIP1().isSet() == true)
		{
			m_modbusTcpSlaveThread1 = new Modbus::TcpSlaveThread(listeningIP1(), *this);
			m_modbusTcpSlaveThread1->start();
		}

		if (listeningIP2().isSet() == true)
		{
			m_modbusTcpSlaveThread2 = new Modbus::TcpSlaveThread(listeningIP2(), *this);
			m_modbusTcpSlaveThread2->start();
		}
	}

	void ModbusTcpSlaveHandler::shutdown()
	{
		if (m_modbusTcpSlaveThread1 != nullptr)
		{
			m_modbusTcpSlaveThread1->stop();
			delete m_modbusTcpSlaveThread1;
			m_modbusTcpSlaveThread1 = nullptr;
		}

		if (m_modbusTcpSlaveThread2 != nullptr)
		{
			m_modbusTcpSlaveThread2->stop();
			delete m_modbusTcpSlaveThread2;
			m_modbusTcpSlaveThread2 = nullptr;
		}

		if (m_appDataServiceClientThread != nullptr)
		{
			m_appDataServiceClientThread->quitAndWait();
			delete m_appDataServiceClientThread;
			m_appDataServiceClientThread = nullptr;
		}
	}

	void ModbusTcpSlaveHandler::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getRequiredSignalsHashes(hashes);
	}

	void ModbusTcpSlaveHandler::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		hashes->clear();
		m_gateway->getEventSignalsHashes(hashes);
	}

	void ModbusTcpSlaveHandler::updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply)
	{
		int statesCount = getStatesReply.appsignalstates_size();

		for(int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = getStatesReply.appsignalstates(i);

			Hash hash = state.hash();

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			std::list<SignalState>& states = it->second;

			for(SignalState& st : states)
			{
				st.value = state.value();
			}
		}

		updateAllRegisters();
	}

	void ModbusTcpSlaveHandler::processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply)
	{
		int statesCount = getStateChangesReply.appsignalstates_size();

		m_hashesToUpdate.clear();

		for(int i = 0; i < statesCount; i++)
		{
			const Proto::AppSignalState& state = getStateChangesReply.appsignalstates(i).curstate();

			Hash hash = state.hash();

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			m_hashesToUpdate.emplace(hash);

			std::list<SignalState>& states = it->second;

			for(SignalState& st : states)
			{
				st.value = state.value();
			}
		}

		updateRegisters(m_hashesToUpdate);
	}

	int ModbusTcpSlaveHandler::modbusDeviceID() const
	{
		return m_gateway->modbusDeviceID();
	}

	int ModbusTcpSlaveHandler::getRegistersValues(int startRegAddr, int regsCount,
												  Modbus::RegisterValue* destBuffer, int maxRegsCount,
												  QThread* thread)
	{
		if (maxRegsCount < regsCount)
		{
			Q_ASSERT(false);
			return 0;
		}

		int copyRegCount = regsCount;

		if (startRegAddr + copyRegCount > m_registers.size())
		{
			copyRegCount = static_cast<int>(m_registers.size()) - startRegAddr;
		}

		int copyDestSizeBytes = copyRegCount * Modbus::REGISTER_SIZE_BYTES;

		m_regsMutex.lock(thread);

		std::memcpy(reinterpret_cast<char*>(destBuffer),
					reinterpret_cast<char*>(m_registers.data() + startRegAddr),
					copyDestSizeBytes);

		m_regsMutex.unlock(thread);

		int fillDestSizeBytes  = (regsCount - copyRegCount) * Modbus::REGISTER_SIZE_BYTES;

		if (fillDestSizeBytes > 0)
		{
			memset(reinterpret_cast<char*>(destBuffer + copyRegCount), 0, fillDestSizeBytes);
		}

		return regsCount * Modbus::REGISTER_SIZE_BYTES;
	}

	bool ModbusTcpSlaveHandler::init()
	{
		const std::map<Address16, std::pair<QString, ModbusFormat>>& mbSignals = m_gateway->modbusSignals();

		auto itLast = mbSignals.rbegin();

		if (itLast == mbSignals.rend())
		{
			return true;
		}

		Address16 maxAddr = itLast->first;

		const ModbusFormat lastSignalFormat = itLast->second.second;

		maxAddr.addWord(lastSignalFormat.registersCount());			// maxAddr here is +1 to real registers count, it is ok

		m_regsMutex.lock();

		m_registers.clear();
		m_registers.resize(maxAddr.offset(), 0);

		m_regsMutex.unlock();

		m_signalsStates.clear();

		for(const auto& [addr16, p] : mbSignals)
		{
			const QString& appSignalID = p.first;
			const ModbusFormat& format = p.second;

			Hash hash = calcHash(appSignalID);

			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				auto [newIt, b] = m_signalsStates.emplace(hash, std::list<SignalState>{});

				it = newIt;
			}

			it->second.emplace_back(format, addr16);
		}

		return true;
	}

	HostAddressPort ModbusTcpSlaveHandler::listeningIP1() const
	{
		return m_gateway->localGatewayIP1();
	}

	HostAddressPort ModbusTcpSlaveHandler::listeningIP2() const
	{
		return m_gateway->localGatewayIP2();
	}

	void ModbusTcpSlaveHandler::updateAllRegisters()
	{
		m_regsMutex.lock();

		for(const auto [hash, states] : m_signalsStates)
		{
			for(const SignalState& state : states)
			{
				updateRegister(state);
			}
		}

		m_regsMutex.unlock();
	}

	void ModbusTcpSlaveHandler::updateRegisters(const std::set<Hash>& hashes)
	{
		m_regsMutex.lock();

		for(Hash hash : hashes)
		{
			auto it = m_signalsStates.find(hash);

			if (it == m_signalsStates.end())
			{
				Q_ASSERT(false);
				continue;
			}

			const std::list<SignalState> states = it->second;

			for(const SignalState& state : states)
			{
				updateRegister(state);
			}
		}

		m_regsMutex.unlock();
	}

	void ModbusTcpSlaveHandler::updateRegister(const SignalState& state)
	{
		// Human readable value for regsStartAddr == 1 in request decremented by 1, i.e. send as 0!
		// So m_registers also indexed from 0
		//
		int regAddr = state.modbusAddress.offset() - 1;

		if (regAddr >= TO_INT(m_registers.size()))
		{
			Q_ASSERT(false);
			return;
		}

		switch(state.format.signalFormat)
		{
		case E::ModbusSignalFormat::DiscreteBit:
			{
				Modbus::RegisterValue& reg = m_registers[regAddr];

				quint16 mask = 1 << state.modbusAddress.bit();

				if (state.reverseBytes)
				{
					mask = reverseUint16(mask);
				}

				if (state.value == 0)
				{
					reg &= (~mask);
				}
				else
				{
					reg |= mask;
				}
			}
			break;

		case E::ModbusSignalFormat::AnalogFloat16:
			{
				Q_ASSERT(false);			// convertion function is unknown
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt16:
			{
				qint16 sint16 = static_cast<qint16>(state.value);

				Modbus::RegisterValue regValue = std::bit_cast<Modbus::RegisterValue>(sint16);

				if (state.reverseBytes)
				{
					regValue = reverseUint32(regValue);
				}

				m_registers[regAddr] = regValue;
			}
			break;

		case E::ModbusSignalFormat::AnalogFloat32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				float fp32 = static_cast<float>(state.value);

				quint32 uint32 = std::bit_cast<quint32>(fp32);

				if (state.reverseBytes)
				{
					uint32 = reverseUint32(uint32);

					m_registers[regAddr] = static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF);
					m_registers[regAddr + 1] = static_cast<Modbus::RegisterValue>((uint32 >> 16) & 0xFFFF);
				}
				else
				{
					m_registers[regAddr] = reverseUint16(static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF));
					m_registers[regAddr + 1] = reverseUint16(static_cast<Modbus::RegisterValue>((uint32 >> 16) & 0xFFFF));
				}
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				qint32 sint32 = static_cast<qint32>(state.value);

				quint32 uint32 = std::bit_cast<quint32>(sint32);

				if (state.reverseBytes)
				{
					uint32 = reverseUint32(uint32);

					m_registers[regAddr] = static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF);
					m_registers[regAddr + 1] = static_cast<Modbus::RegisterValue>((uint32 >> 16) & 0xFFFF);
				}
				else
				{
					m_registers[regAddr] = reverseUint16(static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF));
					m_registers[regAddr + 1] = reverseUint16(static_cast<Modbus::RegisterValue>((uint32 >> 16) & 0xFFFF));
				}
			}
			break;

		default:
			Q_ASSERT(false);
		}
	}

}
