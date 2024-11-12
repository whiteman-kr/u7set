#pragma once

#include "GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"

#include "ModbusProtocol.h"
#include "ModbusSlaveGateway.h"
#include "ModbusTcpSlaveThread.h"

using namespace Modbus;

namespace Gateway
{
	class ModbusSlaveHandler : public Handler
	{
	public:
		ModbusSlaveHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  ModbusSlaveGatewayShared gateway,
						  const AppSignals& appSignals,
						  CircularLoggerShared log,
						  bool logGatewayPackets);

		virtual ~ModbusSlaveHandler();

		virtual void run() override;
		virtual void shutdown() override;

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const override;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const override;

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply) override;
		virtual void processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply) override;

		E::ModbusMode modbusMode() const;
		int modbusDeviceID() const;

		quint8* recvBuffer();
		size_t recvBufferSize() const;
		quint8* sendBuffer();
		size_t sendBufferSize() const;

		int getRegistersValues(int startRegAddr, int regsCount,
							   Modbus::RegisterValue* destBuffer, int maxRegsCount, QThread* thread);

		size_t tcpRequestProcessing(int connNo, const QString& peerAddr,
									const error_code& error,
									size_t bytesReceived);
	private:
		struct SignalState
		{
			SignalState(const ModbusFormat& frmt, const Address16& registerNo);

			ModbusFormat format;
			Address16 regNo;		// regAddr (index in m_registers) == regNo.offset() - 1 !!!

			double value = 0;
		};

	private:
		bool init();

		HostAddressPort listeningIP1() const;
		HostAddressPort listeningIP2() const;

		void updateAllRegisters();
		void updateRegisters(const std::set<Hash>& hashes);
		void updateRegister(const SignalState& state);

		quint16 reverse16(quint16 leValue, E::ModbusByteOrder bo) const;
		quint32 reverse32(quint32 leValue, E::ModbusByteOrder bo) const;

		//

		void logTcpRequest(int connNo, const QString& peerAddr, const error_code& error, size_t bytesReceived);
		void logTcpReply(int connNo, const QString&peerAddr, size_t sendBytes);

		void logAsciiRequest(const error_code& error, size_t bytesReceived);
		void logRtuRequest(const error_code& error, size_t bytesReceived);

		void logAsciiReply(size_t sendBytes);
		void logRtuReply(size_t sendBytes);

		size_t asciiRequestProcessing(size_t bytesReceived);
		size_t rtuRequestProcessing(size_t bytesReceived);

		int onFnReadHoldingRegisters(Modbus::TcpFrame& request);

		Modbus::TcpFrame& getTcpRequestRef();
		Modbus::TcpFrame& getTcpReplyRef();

		int onAsciiFnReadHoldingRegisters(quint16 regsStartAddr, quint16 regsCount);

		bool isHexDigits(const quint8* ptr, int len) const;
		quint8 asciiDecodeXX(const quint8* ptr, bool* ok) const;
		quint16 asciiDecodeXXXX(const quint8* ptr, bool* ok) const;
		quint64 asciiDecode(const quint8* ptr, int len, bool* ok) const;
		quint8 asciiDecodeX(quint8 ch, bool* ok) const;

		quint8* asciiEncodeXX(quint8 v8, quint8* ptr);
		quint8* asciiEncodeXXXX(quint16 v16, quint8* ptr);
		quint8 asciiEncodeX(quint8 ch);

		quint8 nonStandardModbusCrcCalculation(const quint8* ptr, int len);

	private:
		const SoftwareInfo m_softwareInfo;
		HostAddressPort m_appDataService1;
		HostAddressPort m_appDataService2;

		ModbusSlaveGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		AppDataServiceClientThread* m_appDataServiceClientThread = nullptr;

		Modbus::TcpSlaveThread* m_modbusTcpSlaveThread1 = nullptr;
		Modbus::TcpSlaveThread* m_modbusTcpSlaveThread2 = nullptr;

		static inline const size_t RECV_BUFFER_SIZE = 1024;
		quint8 m_recvBuffer[RECV_BUFFER_SIZE];

		static inline const size_t SEND_BUFFER_SIZE = 1024;
		quint8 m_sendBuffer[SEND_BUFFER_SIZE];

		inline static const int ASCII_REG_VALUES_COUNT = 256;

		RegisterValue m_asciiRegValues[ASCII_REG_VALUES_COUNT];

		//

		std::map<Hash, std::list<SignalState>> m_signalsStates;

		SimpleMutex m_regsMutex;
		std::vector<Modbus::RegisterValue> m_registers;		// modbus 16-bit registers
															// values stored in required BE or LE byte order!

		mutable std::set<Hash> m_hashesToUpdate;

		//

		QString m_logStr;

		friend class AppDataServiceClient;
	};

	using ModbusSlaveHandlerShared = std::shared_ptr<ModbusSlaveHandler>;
}
