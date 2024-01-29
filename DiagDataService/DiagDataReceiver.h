#pragma once

#include <queue>

#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/CircularLogger.h"

#include "DiagDataSource.h"
#include "SignalStatesProcessingThread.h"

#include "../asio/include/asio.hpp"

using namespace asio;
using namespace asio::ip;

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

class DiagDataReceiver : public RunOverrideThread
{
	static const int NO_RUP_FRAMES_TIMEOUT = 5;		// in seconds
	static const int MAX_SOCKET_ERROR_COUNT = 3;

public:
	DiagDataReceiver(const HostAddressPort& dataReceivingIP,
					DiagDataSources& diagDataSources,
					DynamicDiagSignalStates& signalStates,
					int processingThreadsCount,
					E::SoftwareRunMode swRunMode,
					CircularLoggerShared log);

	virtual ~DiagDataReceiver() override;

//	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

	const DiagDataSources& diagDataSources() { return m_diagDataSources; }

	CircularLoggerShared log() { return m_log; }

//	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
//									   bool isArchivingQueue,
//									   const QString& description);

//	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

//	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
//										  const std::set<Hash>& hashes);

//	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);

private:
	virtual void run() override;

	void startTimer500ms();
	void onTimer500ms(const error_code& error);

	void clearReceiverStatistics();
	void updateReceiverStatistics();
	void updateDataSourcesStatistics();

	bool createAndBindSocket();
	bool isSocketWorkable() const;
	void closeSocket();
	void startReceive();
	void receivePackets(const error_code& error, std::size_t bytesReceived);

	void requireBufferProcessing(DiagDataSource* source);
	void requireSignalsInvalidation(DiagDataSource* source);

	void startProcessingThreads(StdThreadsGuard& stg);
	void wakeupAllProcessingThreads();

	bool stopIfQuitRequested();

	QString diagDataReceivingIPStr() const;

	void trace_dt(const QString& portID);

private:
	DiagDataSources& m_diagDataSources;
	bool m_isSimulationMode = false;
	int m_processingThreadsCountFromSettings = 0;
	CircularLoggerShared m_log;

	const QThread* m_thisThread = nullptr;

	//

	HostAddressPort m_dataReceivingIP;
	udp::endpoint m_diagDataReceivingIP;

	io_context* m_ioContext = nullptr;
	steady_timer* m_timer = nullptr;
	int m_1second = 0;

	udp::socket* m_socket = nullptr;
	bool m_socketBound = false;
	int m_noReceiveCtr = 0;
	int m_socketErrorCtr = 0;

	static const int RECV_BUFFER_SIZE = sizeof(Rup::SimFrame) + 1;

	//

	int m_writeIndex = 0;
	udp::endpoint m_receiveFromIP[2];
	char m_receiveBuffer[2][RECV_BUFFER_SIZE];

	//

	std::mutex m_packetProcessigRequiredMutex;
	std::condition_variable m_packetProcessingRequiredCondition;
	std::map<DiagDataSource*, bool> m_packetProcessingRequired;		//	source => true	 require buffer processing
																	//	source => false	 require signals invalidation
	friend void processPackets(DiagDataReceiver& receiver, int threadNumber);

	//

	SignalStatesProcessingThread m_statesProcessingThread;

	std::mutex m_statesProcessigRequiredMutex;
	std::condition_variable m_statesProcessingRequiredCondition;
	std::queue<DiagDataSource*> m_statesProcessingRequired;		//	source requires states queue processing

	//

	std::set<quint32> m_unknownDiagDataSourcesIP;

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

void processPackets(DiagDataReceiver& receiver, int threadNumber);
