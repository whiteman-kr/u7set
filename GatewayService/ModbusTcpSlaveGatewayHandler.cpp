#include <string.h>
#include <Network.pb.h>

#include "ModbusTcpSlaveGatewayHandler.h"
#include "Float16.h"

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

	E::ModbusMode ModbusTcpSlaveHandler::modbusMode() const
	{
		return m_gateway->modbusMode();
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

		for(const auto& [hash, states] : m_signalsStates)
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
		int regAddr = state.modbusAddress.offset();

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

				mask = reverse16(mask, state.format.byteOrder);

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
				quint16 f16;

				f16 = encodeFloat16(static_cast<float>(state.value));

				f16 = reverse16(f16, state.format.byteOrder);

				m_registers[regAddr] = f16;
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt16:
			{
				qint16 sint16 = static_cast<qint16>(state.value);
				qint16 uint16 = static_cast<quint16>(sint16);

				uint16 = reverse16(uint16, state.format.byteOrder);

				m_registers[regAddr] = uint16;
			}
			break;

		case E::ModbusSignalFormat::AnalogFloat32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				quint32 uint32 = std::bit_cast<quint32>(static_cast<float>(state.value));

				uint32 = reverse32(uint32, state.format.byteOrder);

				m_registers[regAddr] = static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF);
				m_registers[regAddr + 1] = static_cast<Modbus::RegisterValue>(uint32 >> 16);
			}
			break;

		case E::ModbusSignalFormat::AnalogSInt32:
			{
				if (regAddr + 1 >= TO_INT(m_registers.size()))
				{
					Q_ASSERT(false);
					return;
				}

				quint32 uint32 = std::bit_cast<quint32>(static_cast<qint32>(state.value));

				uint32 = reverse32(uint32, state.format.byteOrder);

				m_registers[regAddr] = static_cast<Modbus::RegisterValue>(uint32 & 0xFFFF);
				m_registers[regAddr + 1] = static_cast<Modbus::RegisterValue>(uint32 >> 16);
			}
			break;

		default:
			Q_ASSERT(false);
		}
	}

	quint16 ModbusTcpSlaveHandler::reverse16(quint16 leValue, E::ModbusByteOrder bo) const
	{
		switch(bo)
		{
		case E::ModbusByteOrder::LE:
		case E::ModbusByteOrder::BE_ByteSwap:
			return leValue;

		case E::ModbusByteOrder::BE:
		case E::ModbusByteOrder::LE_ByteSwap:
			return reverseUint16(leValue);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	quint32 ModbusTcpSlaveHandler::reverse32(quint32 le_Value, E::ModbusByteOrder bo) const
	{
		switch(bo)
		{
		case E::ModbusByteOrder::LE:
			return le_Value;

		case E::ModbusByteOrder::LE_ByteSwap:
			{
				quint16 leValueLow = static_cast<quint16>(le_Value & 0x0000FFFF);
				quint16 leValueHigh = static_cast<quint16>(le_Value >> 16);

				leValueLow = reverseUint16(leValueLow);
				leValueHigh = reverseUint16(leValueHigh);

				le_Value = leValueHigh;
				le_Value <<= 16;
				le_Value |= leValueLow;

				return le_Value;
			}

		case E::ModbusByteOrder::BE_ByteSwap:
			{
				// swap low and high 16 bit words only!
				//
				quint16 leValueHigh = static_cast<quint16>(le_Value >> 16);

				le_Value <<= 16;
				le_Value |= leValueHigh;

				return le_Value;
			}

		case E::ModbusByteOrder::BE:
			return reverseUint32(le_Value);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}
}
