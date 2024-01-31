#pragma once

#include <queue>

#include "SocketIO.h"
#include "CircularLogger.h"

#include "../UtilsLib/SimpleThread.h"
#include "../asio/include/asio.hpp"

#include "OnlineDataSource.h"
//#include "SignalStatesProcessingThread.h"

using namespace asio;
using namespace asio::ip;

//
// RupFramesReceiver is receives a RUP frames from socket and
// push it in OnlineDataSources processing buffers to parse.
// After place RUP frame in buffer wakeup RupFramesParser thread
//
class RupFramesReceiver : public ThreadWithQuit
{
public:
	RupFramesReceiver(const QString& threadName,
						const HostAddressPort& dataReceivingIP,
						OnlineDataSources& onlineDataSources,
						E::SoftwareRunMode swRunMode,
						CircularLoggerShared log);

	~RupFramesReceiver();

	CircularLoggerShared log() { return m_log; }

	void run();

private:
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

	void requireBufferProcessing(OnlineDataSource* source);
	void requireSignalsInvalidation(OnlineDataSource* source);

	bool stopIfQuitRequested();

	QString dataReceivingIPStr() const;

	QString getLogStr(const QString& str);

	void trace_dt(const QString& portID);

private:
	static const int TIMER_PEROIOD_MS = 500;				// in milliseconds
	static const int NO_RUP_FRAMES_TIMEOUT_MS = 3000;		// in milliseconds
	static const int MAX_NO_FRAMES_CTR = NO_RUP_FRAMES_TIMEOUT_MS / TIMER_PEROIOD_MS;
	static const int MAX_SOCKET_ERROR_COUNT = 3;

	//

	const QString m_threadName;
	HostAddressPort m_dataReceivingIP;
	OnlineDataSources& m_onlineDataSources;
	bool m_isSimulationMode = false;
	CircularLoggerShared m_log;

	//

	const QThread* m_thisThread = nullptr;

	//

	io_context* m_ioContext = nullptr;
	steady_timer* m_timer = nullptr;
	udp::socket* m_socket = nullptr;

	//

	bool m_socketBound = false;
	int m_noReceiveCtr = 0;
	int m_socketErrorCtr = 0;
	bool m_1second = false;

	//

	static const int RECV_BUFFER_SIZE = sizeof(Rup::SimFrame) + 1;

	int m_receiveBufIndex = 0;
	udp::endpoint m_receiveFromIP[2];				// indexed by m_receiveBufIndex
	char m_receiveBuffer[2][RECV_BUFFER_SIZE];		// indexed by m_receiveBufIndex

	// statistics variables

	std::atomic<int> m_receivingSpeed = { 0 };				// bytes per second
	std::atomic<int> m_rupFramesReceivingSpeed = { 0 };		// RUP frames per second
	std::atomic<qint64> m_rupFramesCount = { 0 };
	std::atomic<qint64> m_simFramesCount = { 0 };

	std::atomic<qint64> m_errDatagramSize = { 0 };
	std::atomic<qint64> m_errSimVersion = { 0 };
	std::atomic<qint64> m_errUnknownDataSourceIP = { 0 };
	std::atomic<qint64> m_errRupFrameCRC = { 0 };
	std::atomic<qint64> m_errNotExpectedSimPacket = { 0 };

	//

	std::set<quint32> m_unknownDataSourcesIP;

	qint64 m_lastUpdateTime = 0;

	int m_receivedPerSecond = 0;
	int m_rupFramesReceivedPerSecond = 0;
	qint64 m_prevPacketTime = 0;
};


