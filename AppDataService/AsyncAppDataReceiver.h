#pragma once

#include <asio.hpp>

using namespace asio;
using namespace asio::ip;

#include "../OnlineLib/CircularLogger.h"
#include "AppDataSource.h"

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

class AsyncAppDataReceiver : public RunOverrideThread
{
public:
	AsyncAppDataReceiver(const HostAddressPort& dataReceivingIP,
					AppDataSources& appDataSources,
					int processingThreadsCount,
					E::SoftwareRunMode swRunMode,
					CircularLoggerShared log);

	virtual ~AsyncAppDataReceiver() override;

	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

	const AppDataSources& appDataSources() { return m_appDataSources; }

	CircularLoggerShared log() { return m_log; }

private:
	virtual void run() override;

	void startTimer1s();
	void onTimer1s(const error_code& error);

	void clearStatistics();
	void updateReceiverStatistics();
	void updateDataSourcesStatistics();

	bool createAndBindSocket();
	bool isSocketWorkable() const;
	void closeSocket();
	void startReceive();
	void receivePackets(const error_code& error, std::size_t bytesReceived);

	void startProcessingThreads(StdThreadsGuard& stg);
	void wakeupAllProcessingThreads();

	bool stopIfQuitRequested();

	QString appDataReceivingIPStr() const;

private:
	AppDataSources& m_appDataSources;
	bool m_isSimulationMode = false;
	int m_processingThreadsCountFromSettings = 0;
	CircularLoggerShared m_log;

	const QThread* m_thisThread = nullptr;

	//

	udp::endpoint m_appDataReceivingIP;

	io_context* m_ioContext = nullptr;
	steady_timer* m_timer = nullptr;
	udp::socket* m_socket = nullptr;
	bool m_socketBound = false;
	int m_noReceiveCtr = 0;

	static const int RECV_BUFFER_SIZE = sizeof(Rup::SimFrame) + 1;

	//

	int m_writeIndex = 0;
	udp::endpoint m_receiveFromIP[2];
	char m_receiveBuffer[2][RECV_BUFFER_SIZE];

	//

	std::mutex m_receivedConditionMutex;
	std::condition_variable m_packetReceivedCondition;
	std::set<AppDataSource*> m_requireProcessing;

	friend void processPackets(AsyncAppDataReceiver& receiver, int threadNumber);

	//

	std::set<quint32> m_unknownAppDataSourcesIP;

	//

	std::atomic<int> m_receivingRate = { 0 };				// bytes per second
	std::atomic<int> m_udpReceivingRate = { 0 };			// UDP datagrams per second
	std::atomic<int> m_rupFramesReceivingRate = { 0 };		// RUP frames per second
	std::atomic<qint64> m_rupFramesCount = { 0 };
	std::atomic<qint64> m_simFramesCount = { 0 };

	std::atomic<qint64> m_errDatagramSize = { 0 };
	std::atomic<qint64> m_errSimVersion = { 0 };
	std::atomic<qint64> m_errUnknownAppDataSourceIP = { 0 };
	std::atomic<qint64> m_errRupFrameCRC = { 0 };
	std::atomic<qint64> m_errNotExpectedSimPacket = { 0 };

	//

	int m_receivedPerSecond = 0;
	int m_udpReceivedPerSecond = 0;
	int m_rupFramesReceivedPerSecond = 0;
};

void processPackets(AsyncAppDataReceiver& receiver, int threadNumber);
