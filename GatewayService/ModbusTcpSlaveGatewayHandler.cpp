#include "ModbusTcpSlaveGatewayHandler.h"

namespace Gateway
{
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

		m_modbusTcpSlaveThread = new Modbus::TcpSlaveThread(*this);

		m_modbusTcpSlaveThread->start();
	}

	void ModbusTcpSlaveHandler::shutdown()
	{
		if (m_modbusTcpSlaveThread != nullptr)
		{
			m_modbusTcpSlaveThread->stop();
			delete m_modbusTcpSlaveThread;
			m_modbusTcpSlaveThread = nullptr;
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

		hashes->clear();			// no events processing for now
	}

	HostAddressPort ModbusTcpSlaveHandler::listeningAddr() const
	{
		return m_gateway->localGatewayIP1();
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

		memcpy_s(reinterpret_cast<char*>(destBuffer), copyDestSizeBytes,
				 reinterpret_cast<char*>(m_registers.data() + startRegAddr), copyDestSizeBytes);

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

		return true;
	}
}
