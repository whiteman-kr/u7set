#pragma once

#include "GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"

#include "ModbusProtocol.h"
#include "ModbusTcpSlaveGateway.h"
#include "ModbusTcpSlaveThread.h"

namespace Gateway
{
	class ModbusTcpSlaveHandler : public Handler
	{
	public:
		ModbusTcpSlaveHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  ModbusTcpSlaveGatewayShared gateway,
						  const AppSignals& appSignals,
						  CircularLoggerShared log,
						  bool logGatewayPackets);

		~ModbusTcpSlaveHandler();

		virtual void run() override;
		virtual void shutdown() override;

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const override;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const override;

	private:
		bool init();

	private:
		const SoftwareInfo m_softwareInfo;
		HostAddressPort m_appDataService1;
		HostAddressPort m_appDataService2;

		ModbusTcpSlaveGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		AppSignalStates m_states;
		std::atomic_bool m_signalStatesUpdated = { false };

		std::vector<quint16> m_regs;			// modbus 16-bit registers

		AppDataServiceClientThread* m_appDataServiceClientThread = nullptr;
		Modbus::TcpSlaveThread* m_modbusTcpSlaveThread = nullptr;

		friend class AppDataServiceClient;
	};

	using ModbusTcpSlaveHandlerShared = std::shared_ptr<ModbusTcpSlaveHandler>;
}
