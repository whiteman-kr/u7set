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

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply) override;

		HostAddressPort listeningAddr() const;
		int modbusDeviceID() const;

		int getRegistersValues(int startRegAddr, int regsCount,
							   Modbus::RegisterValue* destBuffer, int maxRegsCount, QThread* thread);

	private:
		struct SignalState
		{
			SignalState(const ModbusFormat& frmt, const Address16& addr16);

			ModbusFormat format;
			Address16 modbusAddress;

			bool reverseBytes = false;
			double value = 0;
		};

	private:
		bool init();
		void updateRegisters();

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

		//

		SimpleMutex m_regsMutex;
		std::vector<Modbus::RegisterValue> m_registers;

		std::map<Hash, SignalState> m_signalsStates;

		friend class AppDataServiceClient;
	};

	using ModbusTcpSlaveHandlerShared = std::shared_ptr<ModbusTcpSlaveHandler>;
}
