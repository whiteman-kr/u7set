#pragma once

// #include <QUdpSocket>
#include <queue>
#include <vector>
#include <QReadLocker>

#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../UtilsLib/SimpleThread.h"

#include <SimServiceClientLib/SimServiceClient.h>

// ----------------------------------------------------------------------------------
//
// SimLink class declaration
//
// ----------------------------------------------------------------------------------

class SimLink : public SimpleThreadWorker
{
	Q_OBJECT
public:
	SimLink(const HostAddressPort& simIP, std::shared_ptr<CircularLogger> loggerr);
	~SimLink();

	void pushRequests(std::queue<RvUdpSim::SimRequest>& requests);
	std::queue<RvUdpSim::SimReply> popAllReplies();

signals:
	void repliesReady();

private:
	void onThreadStarted() override;
	void onThreadFinished() override;

	void timerEvent(QTimerEvent* event) override;

	void initTimer();
	void shutdownTimer();

	RvUdpSim::SimReply processGetState();
	RvUdpSim::SimReply processSignalsRead(const RvUdpSim::SignalsReadRequest& request);
	RvUdpSim::SimReply processSignalsWrite(const RvUdpSim::SignalsWriteRequest& request);

private:
	HostAddressPort m_simIP;

	std::shared_ptr<CircularLogger> m_logger;

	//
	std::unique_ptr<Sim::SimServiceClient> m_client;

	QMutex m_lock; 
	std::queue<RvUdpSim::SimRequest> m_requests;
	std::queue<RvUdpSim::SimReply> m_replies;

	QBasicTimer* m_timer = nullptr;
};

class SimLinkThread : public SimpleThread
{
	Q_OBJECT
public:
	SimLinkThread(SimLink* worker);
	
	void pushRequests(std::queue<RvUdpSim::SimRequest> requests);
	std::queue<RvUdpSim::SimReply> popAllReplies();

signals:
	void repliesReady();

private:
	SimLink* m_worker = nullptr;
};
