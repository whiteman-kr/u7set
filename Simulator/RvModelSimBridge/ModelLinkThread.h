#pragma once

#include <QUdpSocket>
#include <queue>
#include <vector>

#include "../../AppSignalLib/TuningDataStorage.h"
#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../UtilsLib/SimpleThread.h"
#include "ModelLinkPacket.h"


// ----------------------------------------------------------------------------------
//
// UdpModelLink class declaration
//
// ----------------------------------------------------------------------------------

class ModelSimBridgeWorker;

class UdpModelLink : public SimpleThreadWorker
{
	Q_OBJECT
public:
	UdpModelLink(const HostAddressPort& listenAddress, int replyPort, std::shared_ptr<CircularLogger> logger);
	~UdpModelLink();

	std::queue<RvUdpSim::SimRequest> popAllRequests();
	void pushReplies(std::queue<RvUdpSim::SimReply>& replies);

private:
	void onThreadStarted() override;
	void onThreadFinished() override;

	void timerEvent(QTimerEvent* event) override;
	void initTimer();
	void shutdownTimer();

	void createSocket();
	void closeSocket();
	bool readSocket();
	bool writeSocket();

private:
	bool processModelPacket_V1(const RvUdpSim::SimulatorBridgePacketHeader_v1* packet, const HostAddressPort& address);

	bool prepareReplyPacket(const RvUdpSim::SimReply& reply, qint64& size);

private slots:
	void onReadyRead();

signals:
	void requestsArrived();

private:
	HostAddressPort m_listenAddress;
	int m_replyPort = 0;

	std::shared_ptr<CircularLogger> m_logger;

	//

	QBasicTimer* m_timer = nullptr;
	QUdpSocket* m_socket = nullptr;
	qint64 m_socketCreateLastTime = 0;

	char* m_requestBuffer = nullptr;
	int m_requestBufferSize = 0;

	char* m_replyBuffer = nullptr;
	int m_replyBufferSize = 0;

	// statistics
	//
	qint64 m_errReadSocket = 0;
	qint64 m_errRequestMarker = 0;
	qint64 m_errRequestSize = 0;
	qint64 m_errCrc = 0;
	qint64 m_errVersion = 0;
	qint64 m_errWriteSocket = 0;
	qint64 m_errReplySize = 0;


	QMutex m_mutex;
	std::queue<RvUdpSim::SimRequest> m_requests;
	std::queue<RvUdpSim::SimReply> m_replies;
};

class UdpModelLinkThread : public SimpleThread
{
	Q_OBJECT
public:
	UdpModelLinkThread(UdpModelLink* worker);

	std::queue<RvUdpSim::SimRequest> popAllRequests();
	void pushReplies(std::queue<RvUdpSim::SimReply> replies);

signals:
	void requestsArrived();

private:
	UdpModelLink* m_worker = nullptr;
};
