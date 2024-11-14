#pragma once

#include <asio.hpp>

#include "GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"

#include "ModbusProtocol.h"
#include "ModbusSlaveGateway.h"

using namespace Modbus;
using namespace asio;

namespace Modbus
{
	class TcpSlaveThread;
	class UdpSlaveThread;
}

namespace Gateway
{
	struct MbshProcData				// ModbusSlaveHandler processing data
	{
		int connNo = 0;
		QString peerAddr;
		error_code error;

		quint8* recvBuffer = nullptr;
		size_t recvBufferSize = 0;
		size_t bytesReceived = 0;

		quint8* sendBuffer = nullptr;
		size_t sendBufferSize = 0;
		size_t sendBytes = 0;
	};

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

		int getRegistersValues(int regsStartAddr, int regsCount,
							   Modbus::RegisterValue* destBuffer, int maxRegsCount, QThread* thread);

		size_t tcpRequestProcessing(MbshProcData& mpd);
		size_t asciiRequestProcessing(MbshProcData& mpd);
		size_t rtuRequestProcessing(MbshProcData& mpd);

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

		void logTcpRequest(MbshProcData& mpd);
		void logTcpReply(MbshProcData& mpd);

		void logAsciiRequest(MbshProcData& mpd);
		void logAsciiReply(MbshProcData& mpd);

		void logRtuRequest(MbshProcData& mpd);
		void logRtuReply(MbshProcData& mpd);

		size_t onFnReadHoldingRegisters(MbshProcData& mpd);

		Modbus::TcpFrame& getTcpRequestRef(MbshProcData& mpd);
		Modbus::TcpFrame& getTcpReplyRef(MbshProcData& mpd);

		size_t onAsciiFnReadHoldingRegisters(quint16 regsStartAddr, quint16 regsCount, MbshProcData& mpd);

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

		Modbus::TcpSlaveThread* m_tcpSlaveThread1 = nullptr;
		Modbus::TcpSlaveThread* m_tcpSlaveThread2 = nullptr;

		Modbus::UdpSlaveThread* m_udpSlaveThread1 = nullptr;
		Modbus::UdpSlaveThread* m_udpSlaveThread2 = nullptr;

		inline static const int BIN_DATA_SIZE = 1024;
		quint8 m_binData[BIN_DATA_SIZE];

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
