#include <algorithm>

#include "../../OnlineLib/CircularLogger.h"
#include "../../UtilsLib/Crc.h"
#include "../../UtilsLib/WUtils.h"

#include <HardwareLib/DataProtocols.h>

#include "ModelLinkPacket.h"
#include "SimLinkThread.h"

using namespace RvUdpSim;

// -------------------------------------------------------------------------
//
//	SimLink class implementaton
//
// -------------------------------------------------------------------------

SimLink::SimLink(const HostAddressPort& simIP, std::shared_ptr<CircularLogger> logger) :
	m_simIP(simIP),
	m_logger(logger)
{
}

SimLink::~SimLink() {}

void SimLink::pushRequests(std::queue<RvUdpSim::SimRequest>& requests) 
{
	QMutexLocker l(&m_lock);

	while (requests.empty() == false)
	{
		m_requests.push(requests.front());
		requests.pop();
	}
}

std::queue<RvUdpSim::SimReply> SimLink::popAllReplies()
{
	std::queue<RvUdpSim::SimReply> result;

	QMutexLocker l(&m_lock);
	result = std::move(m_replies);
	m_replies = {};

	return result;
}

void SimLink::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger,
				  QString(tr("Simulator Communication thread is started with Simulator address %1.")).arg(m_simIP.addressPortStr()));

	m_client = std::make_unique<Sim::SimServiceClient>(m_simIP.toString());

	initTimer();
}

void SimLink::onThreadFinished()
{
	shutdownTimer();

	m_client.reset();

	DEBUG_LOG_MSG(m_logger,
				  QString(tr("Simulator Communication thread is finished with Simulator address %1.")).arg(m_simIP.addressPortStr()));
}

void SimLink::initTimer()
{
	Q_ASSERT(m_timer == nullptr);

	m_timer = new QBasicTimer();

	m_timer->start(1, Qt::PreciseTimer, this);
}

void SimLink::shutdownTimer()
{
	TEST_PTR_RETURN(m_timer);

	m_timer->stop();

	delete m_timer;

	m_timer = nullptr;
}

void SimLink::timerEvent(QTimerEvent* event)
{
	TEST_PTR_RETURN(m_timer);

	if (event->timerId() != m_timer->timerId())
	{
		return;
	}

	// Socket is created
	//
	int count = 0;
	
	do
	{
		count++;
		
		QMutexLocker l(&m_lock);
		if (m_requests.empty() == true) 
		{
			break;
		}

		// Take the top request
		//
		SimRequest request = m_requests.front();
		m_requests.pop();

		// Limit reply queue suze to 100 items
		//
		while (m_replies.size() > 100) 
		{
			m_replies.pop();
		}

		l.unlock();

		SimReply reply = {.type = request.type, .addressTo = request.addressFrom};

		// Process the request
		//
		switch (request.type)
		{
		case SGW_SIGNAL_READ:
			{
				Q_ASSERT(request.readRequest.has_value());
				reply.readReply = processSignalsRead(request.readRequest.value());
			}
			break;
		case SGW_SIGNAL_WRITE:
			{
				Q_ASSERT(request.writeRequest.has_value());
				reply.writeReply = processSignalsWrite(request.writeRequest.value());
			}
			break;
		case SGW_COMMAND_GET_STATE:
			{
				reply.stateReply = processGetState();
			}
			break;
		case SGW_COMMAND_START:
		case SGW_COMMAND_STOP:
		case SGW_COMMAND_PAUSE:
		case SGW_COMMAND_RESUME:
			{
				reply.stateReply = processSimulatorControl(request.type);
			}
			break;
		default:
			Q_ASSERT(false);
			DEBUG_LOG_ERR(m_logger, QString(tr("SimLink: unknown request type: %1.")).arg(request.type));
			
			continue;
		}

		QMutexLocker wl(&m_lock);
		m_replies.push(std::move(reply));

	} while (count < 100);

	// Signalize that we have some replies
	//
	QMutexLocker l(&m_lock);
	if (m_replies.empty() == false)
	{
		l.unlock();
		emit repliesReady();
	}
}

SignalsReadReply SimLink::processSignalsRead(const SignalsReadRequest& request)
{
	std::vector<SignalState> replyStates;
	auto result = m_client->GetSignalState(request.hashes);

	if (result.has_value() == false)
	{
		replyStates.reserve(request.hashes.size());
		for (int i = 0; i < request.hashes.size(); i++)
		{
			replyStates.push_back({.hash = request.hashes[i], .time = 0, .value = 0, .flags = {.all = 0}});
		}
	}
	else
	{
		const std::vector<::AppSignalState>& states = result.value();
		for (const ::AppSignalState& state : states)
		{
			SignalState st = {.hash = state.hash(),
							  .time = state.time().local.timeStamp,
							  .value{.fValue = (float)state.value()}, // Check the type!!!
							  .flags = {.all = state.m_flags.all & 0xffff}};

			replyStates.push_back(st);
		}
	}

	SignalsReadReply reply = {replyStates};
	return reply;
}

SignalsWriteReply SimLink::processSignalsWrite(const SignalsWriteRequest& request)
{
	std::vector<ErrorCode> errorCodes;
	errorCodes.resize(request.values.size());

	std::vector<Sim::SimServiceClient::OverrideSignalPair> overrideSignals;

	auto signalParams = m_client->GetSignalParam(request.hashes);
	if (signalParams.has_value() == false)
	{
		std::fill(errorCodes.begin(), errorCodes.end(), ErrorCode::SignalNotFound);
	}
	else
	{
		const std::vector<::AppSignalParam>& params = signalParams.value();

		if (params.size() != request.values.size())
		{
			Q_ASSERT(false);
			std::fill(errorCodes.begin(), errorCodes.end(), ErrorCode::SignalNotFound);
		}
		else
		{
			int i = 0;
			for (const ::AppSignalParam& sp : params)
			{
				Sim::SimServiceClient::OverrideSignalPair osp;
				osp.appSignalId = sp.appSignalId();
				osp.value = request.values[i++].fValue; // Check the type!!!
				overrideSignals.push_back(osp);
			}

			auto result = m_client->OverrideSignals(overrideSignals);
			if (result.has_value() == false)
			{
				std::fill(errorCodes.begin(), errorCodes.end(), ErrorCode::SignalNotFound);
			}
			else
			{
				std::fill(errorCodes.begin(), errorCodes.end(), ErrorCode::Success);
			}
		}
	}

	SignalsWriteReply reply = {.errorCodes = errorCodes};
	return reply;
}

SimulatorStateReply SimLink::processGetState()
{
	SimulatorStateReply reply;

	QByteArray payload = "Hello, World!";
	auto pingResult = m_client->Ping(payload);

	if (pingResult.has_value() == false)
	{
		reply = {RvUdpSim::NoConnection, RvUdpSim::Unavailable};
	}
	else
	{
		if (payload != pingResult)
		{
			reply = {RvUdpSim::Success, RvUdpSim::Unavailable};
		}
		else
		{
			auto status = m_client->GetStatus();
			if (status.has_value() == false)
			{
				reply = {RvUdpSim::Success, RvUdpSim::Unavailable};
			}
			else
			{
				reply = {RvUdpSim::Success, RvUdpSim::Unavailable};

				switch (status->state)
				{
				case Sim::SimServiceClient::STATE_STOPPED:
					reply = {RvUdpSim::Success, RvUdpSim::Stopped};
					break;
				case Sim::SimServiceClient::STATE_RUNNING:
					reply = {RvUdpSim::Success, RvUdpSim::Running};
					break;
				case Sim::SimServiceClient::STATE_PAUSED:
					reply = {RvUdpSim::Success, RvUdpSim::Paused};
					break;
				default:
					Q_ASSERT(false);
					reply = {RvUdpSim::Success, RvUdpSim::Unavailable};
				}
			}
		}
	}

	return reply;
}

SimulatorStateReply SimLink::processSimulatorControl(int command)
{
	SimulatorStateReply reply;

	switch(command)
	{
	case SGW_COMMAND_START:
	case SGW_COMMAND_RESUME:
		{
			auto status = m_client->CommandStart();
			if (status.has_value() == false)
			{
				reply = {RvUdpSim::NoConnection, RvUdpSim::Unavailable};
			}
			else
			{
				reply = {RvUdpSim::Success, RvUdpSim::Running};
			}
		}
		break;
	case SGW_COMMAND_STOP:
		{
			auto status = m_client->CommandStop();
			if (status.has_value() == false)
			{
				reply = {RvUdpSim::NoConnection, RvUdpSim::Unavailable};
			}
			else
			{
				reply = {RvUdpSim::Success, RvUdpSim::Stopped};
			}
		}
		break;
	case SGW_COMMAND_PAUSE:
		{
			auto status = m_client->CommandPause();
			if (status.has_value() == false)
			{
				reply = {RvUdpSim::NoConnection, RvUdpSim::Unavailable};
			}
			else
			{
				reply = {RvUdpSim::Success, RvUdpSim::Paused};
			}
		}
		break;
	default:
		Q_ASSERT(false);
		reply = {RvUdpSim::NoConnection, RvUdpSim::Unavailable};
	}

	return reply;
} 
// ----------------------------------------------------------------------------------
//
// SimLinkThread class implementation
//
// ----------------------------------------------------------------------------------

SimLinkThread::SimLinkThread(SimLink* worker) :
	m_worker(worker)
{
	addWorker(worker);
	
	connect(m_worker,
			&SimLink::repliesReady,
			this,
			[this]()
			{
				emit repliesReady();
			});
}

void SimLinkThread::pushRequests(std::queue<RvUdpSim::SimRequest> requests)
{
	m_worker->pushRequests(requests);
}

std::queue<RvUdpSim::SimReply> SimLinkThread::popAllReplies()
{
	return m_worker->popAllReplies();
}