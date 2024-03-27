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
		virtual void processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply) override;

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

		HostAddressPort listeningIP1() const;
		HostAddressPort listeningIP2() const;

		void updateAllRegisters();
		void updateRegisters(const std::set<Hash>& hashes);
		void updateRegister(const SignalState& state);

	private:
		const SoftwareInfo m_softwareInfo;
		HostAddressPort m_appDataService1;
		HostAddressPort m_appDataService2;

		ModbusTcpSlaveGatewayShared m_gateway;
		const AppSignals& m_appSignals;

//		AppSignalStates m_states;
//		std::atomic_bool m_signalStatesUpdated = { false };

		AppDataServiceClientThread* m_appDataServiceClientThread = nullptr;

		Modbus::TcpSlaveThread* m_modbusTcpSlaveThread1 = nullptr;
		Modbus::TcpSlaveThread* m_modbusTcpSlaveThread2 = nullptr;

		//

		std::map<Hash, std::list<SignalState>> m_signalsStates;

		SimpleMutex m_regsMutex;
		std::vector<Modbus::RegisterValue> m_registers;				// modbus 16-bit registers

		mutable std::set<Hash> m_hashesToUpdate;

		friend class AppDataServiceClient;
	};

	using ModbusTcpSlaveHandlerShared = std::shared_ptr<ModbusTcpSlaveHandler>;
}
