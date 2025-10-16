#pragma once

#include <queue>

#include "../OnlineLib/CircularLogger.h"
#include "AppDataSource.h"
#include "SignalStatesProcessingThread.h"

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>

//
// AppDataReceiver is receives RUP datagrams and push it in AppDataSource's queues
//

class StdThreadsGuard
{
public:
	StdThreadsGuard();
	~StdThreadsGuard();

	void append(std::thread& thread);

private:
	std::map<std::size_t, std::thread> m_threads;
};

class AppDataReceiver : public RunOverrideThread
{
	static const int NO_RUP_FRAMES_TIMEOUT = 5;		// in seconds
	static const int MAX_SOCKET_ERROR_COUNT = 3;

public:
	AppDataReceiver(const HostAddressPort& dataReceivingIP,
					AppDataSources& appDataSources,
					DynamicAppSignalStates& signalStates,
					int processingThreadsCount,
					E::SoftwareRunMode swRunMode,
					CircularLoggerShared log);

	virtual ~AppDataReceiver() override;

	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

	const AppDataSources& appDataSources() { return m_appDataSources; }

	CircularLoggerShared log() { return m_log; }

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
									   bool isArchivingQueue,
									   const QString& description);

	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
										  const std::set<Hash>& hashes);

	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);

private:
	virtual void run() override;

	void startTimer500ms();
	void onTimer500ms(const asio::error_code& error);

	void clearReceiverStatistics();
	void updateReceiverStatistics();
	void updateDataSourcesStatistics();

	bool createAndBindSocket();
	bool isSocketWorkable() const;
	void closeSocket();
	void startReceive();
	void receivePackets(const asio::error_code& error, std::size_t bytesReceived);

	void requireBufferProcessing(AppDataSource* source);
	void requireSignalsInvalidation(AppDataSource* source);

	void startProcessingThreads(StdThreadsGuard& stg);
	void wakeupAllProcessingThreads();

	bool stopIfQuitRequested();

	QString appDataReceivingIPStr() const;

	void trace_dt(const QString& portID);

private:
	AppDataSources& m_appDataSources;
	bool m_isSimulationMode = false;
	int m_processingThreadsCountFromSettings = 0;
	CircularLoggerShared m_log;

	//

	HostAddressPort m_dataReceivingIP;
	asio::ip::udp::endpoint m_appDataReceivingIP;

	asio::io_context* m_ioContext = nullptr;
	asio::steady_timer* m_timer = nullptr;
	int m_1second = 0;

	asio::ip::udp::socket* m_socket = nullptr;
	bool m_socketBound = false;
	int m_noReceiveCtr = 0;
	int m_socketErrorCtr = 0;

	//

	int m_writeIndex = 0;
	asio::ip::udp::endpoint m_receiveFromIP[2];

	static constexpr std::size_t RECV_BUFFER_SIZE = std::max(sizeof(Rup::SimFrame), sizeof(Rup::Frame));

	std::array<std::byte, RECV_BUFFER_SIZE> m_receiveBuffer[2];

	//

	std::mutex m_packetProcessigRequiredMutex;
	std::condition_variable m_packetProcessingRequiredCondition;
	std::map<AppDataSource*, bool> m_packetProcessingRequired;		//	source => true	 require buffer processing
																	//	source => false	 require signals invalidation
	friend void processPackets(AppDataReceiver& receiver, int threadNumber);

	//

	SignalStatesProcessingThread m_statesProcessingThread;

	std::mutex m_statesProcessigRequiredMutex;
	std::condition_variable m_statesProcessingRequiredCondition;
	std::queue<AppDataSource*> m_statesProcessingRequired;		//	source requires states queue processing

	//

	std::set<quint32> m_unknownAppDataSourcesIP;

	//

	std::atomic<int> m_receivingSpeed = { 0 };				// bytes per second
	std::atomic<int> m_rupFramesReceivingSpeed = { 0 };		// RUP frames per second
	std::atomic<qint64> m_rupFramesCount = { 0 };
	std::atomic<qint64> m_simFramesCount = { 0 };

	std::atomic<qint64> m_errDatagramSize = { 0 };
	std::atomic<qint64> m_errSimVersion = { 0 };
	std::atomic<qint64> m_errUnknownAppDataSourceIP = { 0 };
	std::atomic<qint64> m_errRupFrameCRC = { 0 };
	std::atomic<qint64> m_errNotExpectedSimPacket = { 0 };

	//

	qint64 m_lastUpdateTime = 0;

	int m_receivedPerSecond = 0;
	int m_rupFramesReceivedPerSecond = 0;
	qint64 m_prevPacketTime = 0;

	//

	friend class SignalStatesProcessingThread;
};

void processPackets(AppDataReceiver& receiver, int threadNumber);
