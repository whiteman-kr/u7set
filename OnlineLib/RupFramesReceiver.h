#pragma once

#include <queue>

#include "SocketIO.h"
#include "CircularLogger.h"
#include <HardwareLib/DataProtocols.h>
#include "../UtilsLib/WUtils.h"
#include <CommonLib/HostAddressPort.h>

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>

using namespace asio;
using namespace asio::ip;

#pragma warning(push)
#pragma warning(disable: 4324)

class BaseOnlineDataSources;

//
// RupFramesReceiver is receives a RUP frames from socket and
// push it in OnlineDataSources processing buffers to parse.
//
class RupFramesReceiver
{
public:
	RupFramesReceiver(	const HostAddressPort& dataReceivingIP,
						E::SoftwareRunMode swRunMode,
						BaseOnlineDataSources& onlineDataSources,
						CircularLoggerShared log);

	~RupFramesReceiver();

	CircularLoggerShared log();

	void run(std::stop_token stopToken);

private:
	void startTimer500ms();
	void onTimer500ms(const error_code& error);

	void startReceiveRupFrames();
	void onRupFrameReceive(const error_code& error, std::size_t bytesReceived);

	bool createAndBindSocket();
	bool isSocketWorkable() const;
	void closeSocket();

	bool stopIfQuitRequested();

	void clearReceiverStatistics();
	void updateReceiverStatistics();

	QString dataReceivingIPStr() const;
	QString getLogStr(const QString& str);
//	void trace_dt(const QString& portID);

private:
	inline static const int TIMER_PERIOD_MS = 500;				// in milliseconds
	inline static const int NO_RUP_FRAMES_TIMEOUT_MS = 3000;		// in milliseconds
	inline static const int MAX_NO_FRAMES_CTR = NO_RUP_FRAMES_TIMEOUT_MS / TIMER_PERIOD_MS;
	inline static const int MAX_SOCKET_ERROR_COUNT = 3;

	//

	const QString m_threadName;
	HostAddressPort m_dataReceivingIP;
	BaseOnlineDataSources& m_onlineDataSources;
	bool m_isSimulationMode = false;
	CircularLoggerShared m_log;

	//

	const QThread* m_thisThread = nullptr;

	//

	io_context* m_ioContext = nullptr;
	steady_timer* m_timer = nullptr;
	udp::socket* m_socket = nullptr;
	std::stop_token m_stopToken;

	//

	bool m_socketBound = false;
	int m_noReceiveCtr = 0;
	int m_socketErrorCtr = 0;
	bool m_1second = false;

	//

	int m_receiveBufIndex = 0;

	// selected by m_receiveBufIndex
	//
	udp::endpoint m_receiveFromIP0;
	udp::endpoint m_receiveFromIP1;

	inline static const int RECV_BUFFER_SIZE = ROUND_TO_CACHE_LINE_SIZE(sizeof(Rup::SimFrame));

	CACHE_ALIGNED char m_receiveBuffer0[RECV_BUFFER_SIZE];
	CACHE_ALIGNED char m_receiveBuffer1[RECV_BUFFER_SIZE];

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

#pragma warning(pop)
