#pragma once

#include <asio/error_code.hpp>

#include "../GatewayLib/ModbusSlaveGateway.h"

#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "ModbusProtocol.h"
#include "../OnlineLib/GrpcAdsClient.h"

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

		virtual void onAppDataSrvConnected() override;
		virtual void onAppDataSrvDisconnected() override;
		virtual void planNextPreparedRequest(PreparedRequest& request) override;

		void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply);
		void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply);
		void invalidateSignals();

		E::ModbusMode modbusMode() const;
		int modbusDeviceID() const;

		int getRegistersValues(int regsStartAddr, int regsCount,
							   Modbus::RegisterValue* destBuffer, int maxRegsCount);

		size_t tcpRequestProcessing(MbshProcData& mpd);
		size_t asciiRequestProcessing(MbshProcData& mpd);
		size_t rtuRequestProcessing(MbshProcData& mpd);

	private:
		class SignalState
		{
		public:
			SignalState(const ModbusFormat& frmt, const Address16& registerNo, bool isConst, double constValue);

			const ModbusFormat& format() const;
			const Address16& regNo() const;
			double value() const;
			bool isConst() const;

			void setValue(double value);

		private:
			ModbusFormat m_format;
			Address16 m_regNo;			// regAddr (index in m_registers) == regNo.offset() - 1 !!!
			double m_value = 0;
			bool m_isConst = false;
		};

	private:
		bool init();

		virtual void runAppDataSrvClient() override;

		virtual void prepareRequests() override;

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

		size_t onAsciiFn03ReadHoldingRegisters(Message& msg, MbshProcData& mpd, quint16 regsStartAddrOffset);

		bool convertAsciiToBin(const quint8* asciiPtr, size_t asciiLen,
							   quint8* binPtr, size_t* binLen);

		quint8 calcCrc(const quint8* data, size_t dataLength) const;
		quint8 crcKzUik(const quint8* data, size_t dataLength) const;

		bool isHexDigits(const quint8* ptr, int len) const;
		quint8 asciiDecodeXX(const quint8* ptr, bool* ok) const;
		quint16 asciiDecodeXXXX(const quint8* ptr, bool* ok) const;
		quint64 asciiDecode(const quint8* ptr, int len, bool* ok) const;
		quint8 asciiDecodeX(quint8 ch, bool* ok) const;

		quint8* asciiEncodeXX(quint8 v8, quint8* ptr);
		quint8* asciiEncodeXXXX(quint16 v16, quint8* ptr);
		quint8 asciiEncodeX(quint8 ch);

	private:
		ModbusSlaveGatewayShared m_gateway;

		Modbus::TcpSlaveThread* m_tcpSlaveThread1 = nullptr;
		Modbus::TcpSlaveThread* m_tcpSlaveThread2 = nullptr;

		Modbus::UdpSlaveThread* m_udpSlaveThread1 = nullptr;
		Modbus::UdpSlaveThread* m_udpSlaveThread2 = nullptr;

		inline static const int BIN_DATA_SIZE = 1024;
		quint8 m_binData[BIN_DATA_SIZE];

		//

		std::map<Hash, std::list<SignalState>> m_signalsStates;

		SpinLock m_regsMutex;
		std::vector<Modbus::RegisterValue> m_registers;		// modbus 16-bit registers
															// values stored in required BE or LE byte order!

		mutable std::set<Hash> m_hashesToUpdate;

		//

		QString m_logStr;

		friend class AppDataServiceClient;
	};

	using ModbusSlaveHandlerShared = std::shared_ptr<ModbusSlaveHandler>;

	class ModbusAppSignalStateUpdater : public IAppSignalStateUpdater
	{
	public:
		ModbusAppSignalStateUpdater(ModbusSlaveHandler& handler);

		virtual void adsConnected() override;
		virtual void adsDisconnected() override;

		virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override;
		virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override;
		virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override;

	private:
		ModbusSlaveHandler& m_handler;
	};

}
