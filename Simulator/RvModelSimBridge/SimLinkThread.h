#pragma once

#include <queue>
#include <QReadLocker>

#include "../../OnlineLib/CircularLogger.h"
#include "../../UtilsLib/SimpleThread.h"

#include <SimServiceClientLib/SimServiceClient.h>
#include "ModelLinkPacket.h"

// ----------------------------------------------------------------------------------
//
// SimLink class declaration
//
// ----------------------------------------------------------------------------------

class SimLink : public SimpleThreadWorker
{
	Q_OBJECT
public:
	SimLink(const HostAddressPort& simIP, std::shared_ptr<CircularLogger> appLogger, std::shared_ptr<CircularLogger> simLogger);
	~SimLink();

	void pushRequests(std::queue<SimRequest>& requests);
	std::queue<SimReply> popAllReplies();

signals:
	void repliesReady();

private:
	void onThreadStarted() override;
	void onThreadFinished() override;

	void initTimer();
	void shutdownTimer();
	void timerEvent(QTimerEvent* event) override;

	SignalReadReplyRef processSignalsRead(const SignalReadRequestRef& request);
	SignalWriteReplyRef processSignalsWrite(const SignalWriteRequestRef& request);
	SimulatorStateReply processGetState();
	SimulatorStateReply processSimulatorControl(int command);

private:
	HostAddressPort m_simIP;

	std::shared_ptr<CircularLogger> m_appLogger;
	std::shared_ptr<CircularLogger> m_simLogger;

	//
	std::unique_ptr<Sim::SimServiceClient> m_client;

	QMutex m_lock;  // m_requests and m_replies lock
	std::queue<SimRequest> m_requests;
	std::queue<SimReply> m_replies;

	QBasicTimer* m_timer = nullptr;
};

// ----------------------------------------------------------------------------------
//
// SimLinkThread class declaration
//
// ----------------------------------------------------------------------------------

class SimLinkThread : public SimpleThread
{
	Q_OBJECT
public:
	SimLinkThread(SimLink* worker);
	
	void pushRequests(std::queue<SimRequest> requests);
	std::queue<SimReply> popAllReplies();

signals:
	void repliesReady();

private:
	SimLink* m_worker = nullptr;
};
