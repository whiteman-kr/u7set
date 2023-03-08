#pragma once

#include <asio.hpp>

using namespace asio;
using namespace asio::ip;

#include "../OnlineLib/CircularLogger.h"
#include "AppDataSource.h"

//
// AppDataReceiver is receives RUP datagrams and push it in AppDataSource's queues
//

class AsyncAppDataReceiver : public RunOverrideThread
{
public:
	AsyncAppDataReceiver(const HostAddressPort& dataReceivingIP,
					const AppDataSourcesIP& appDataSourcesIP,
					E::SoftwareRunMode swRunMode,
					CircularLoggerShared log);

	virtual ~AsyncAppDataReceiver() override;

	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

	const AppDataSourcesIP& appDataSourcesIP() { return m_appDataSourcesIP; }

private:
	virtual void run() override;

	void startTimer1s();
	void onTimer1s(const error_code& error);

	void clearStatistics();
	void updateStatistics();

	bool createAndBindSocket();
	bool isSocketWorkable() const;
	void closeSocket();
	void startReceive();
	void receivePackets(const error_code& error, std::size_t bytesReceived);

	void startAppDataProcessingThreads();

	QString appDataReceivingIPStr() const;

private:
	const AppDataSourcesIP& m_appDataSourcesIP;
	CircularLoggerShared m_log;
	bool m_isSimulationMode = false;

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

	std::atomic<int> m_receivedPerSecond = 0;
	std::atomic<int> m_udpReceivedPerSecond = 0;
	std::atomic<int> m_rupFramesReceivedPerSecond = 0;
};
