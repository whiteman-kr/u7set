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

SimLink::SimLink(const HostAddressPort& simIP, std::shared_ptr<CircularLogger> appLogger, std::shared_ptr<CircularLogger> simLogger) :
	m_simIP(simIP),
	m_appLogger(appLogger),
	m_simLogger(simLogger)
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
	DEBUG_LOG_MSG(m_appLogger,
				  QString(tr("Simulator Communication thread is started with Simulator address %1.")).arg(m_simIP.addressPortStr()));

	m_client = std::make_unique<Sim::SimServiceClient>(m_simIP.toString());

	initTimer();
}

void SimLink::onThreadFinished()
{
	shutdownTimer();

	m_client.reset();

	DEBUG_LOG_MSG(m_appLogger,
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
			DEBUG_LOG_ERR(m_appLogger, QString(tr("SimLink: unknown request type: %1.")).arg(request.type));

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
	// Fill reply with empty states
	//
	std::vector<SignalState> replyStates;
	replyStates.reserve(request.hashes.size());

	for (int i = 0; i < request.hashes.size(); i++)
	{
		replyStates.push_back({.hash = request.hashes[i], .time = 0, .value = 0, .flags = {.all = 0}});
	}

	//
	auto allParams = m_client->GetSignalParam(request.hashes);
	auto allStates = m_client->GetSignalState(request.hashes);

	if (allParams.has_value() == false || allStates.has_value() == false)
	{
		DEBUG_LOG_MSG(m_simLogger, tr("Read signals (%1 hashes): no connection to Simulator.").arg(request.hashes.size()));
		return {replyStates};
	}

	const std::vector<::AppSignalParam>& params = allParams.value();
	const std::vector<::AppSignalState>& states = allStates.value();

	if (params.size() != states.size() || params.size() != request.hashes.size())
	{
		DEBUG_LOG_MSG(m_simLogger,
					  tr("Read signals (%1 hashes): result size mismatch (%2 params, %3 states).")
						  .arg(request.hashes.size())
						  .arg(params.size())
						  .arg(states.size()));
		return {replyStates};
	}

	for (std::size_t i = 0; i < params.size(); ++i)
	{
		const auto& param = params[i];
		const auto& state = states[i];

		if (param.appSignalId().isEmpty() == true)
		{
			DEBUG_LOG_MSG(m_simLogger, tr("Read signal: hash = '%1': signal not found.").arg(request.hashes[i]));
			replyStates.push_back({.hash = request.hashes[i], .time = 0, .value = 0, .flags = {.all = 0}});
			continue;
		}

		SignalValue sv = {0};
		if (param.isDiscrete())
		{
			sv.bValue = state.value() == 0 ? 0 : 1;
		}
		else
		{
			if (param.isAnalog())
			{
				if (param.analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
				{
					sv.fValue = state.value();
				}
				else
				{
					sv.iValue = state.value();
				}
			}
		}

		// Write the result state
		//
		replyStates[i] = {.hash = state.hash(),
						  .time = state.time().local.timeStamp,
						  .value = sv,
						  .flags = {.all = static_cast<unsigned short>(state.m_flags.all & 0xffff)}};
	}

	return {replyStates};
}

SignalsWriteReply SimLink::processSignalsWrite(const SignalsWriteRequest& request)
{
	std::vector<ErrorCode> errorCodes;
	errorCodes.resize(request.values.size());

	for (int i = 0; i < request.hashes.size(); i++)
	{
		Hash hash = request.hashes[i];

		std::vector<Sim::SimServiceClient::OverrideSignalPair> overrideSignals;

		std::vector<Hash> h;
		h.push_back(hash);

		auto signalParam = m_client->GetSignalParam(h);
		if (signalParam.has_value() == false)
		{
			DEBUG_LOG_MSG(m_simLogger, tr("Override signal: hash = '%1': no connection to Simulator.").arg(hash));
			errorCodes[i] = ErrorCode::NoConnection;
			continue;
		}

		const ::AppSignalParam& asp = signalParam.value()[0];
		if (asp.appSignalId().isEmpty() == true)
		{
			DEBUG_LOG_MSG(m_simLogger, tr("Override signal: hash = '%1': signal not found.").arg(hash));
			errorCodes[i] = ErrorCode::SignalNotFound;
			continue;
		}

		Sim::SimServiceClient::OverrideSignalPair osp;
		osp.appSignalId = asp.appSignalId();

		if (asp.isDiscrete())
		{
			bool bValue = request.values[i].bValue;
			
			osp.value = bValue;
			DEBUG_LOG_MSG(m_simLogger, tr("Override signal: id = '%1', bValue = '%2'").arg(asp.appSignalId()).arg(bValue));
		}
		else
		{
			if (asp.isAnalog() == true)
			{
				if (asp.analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
				{
					double fValue = request.values[i].fValue;
					if (fValue < asp.lowEngineeringUnits() || fValue > asp.highEngineeringUnits())
					{
						DEBUG_LOG_MSG(m_simLogger,
									  tr("Override signal: id = '%1', fValue = '%2' is out of range (%3..%4)")
										  .arg(asp.appSignalId())
										  .arg(fValue)
										  .arg(asp.lowEngineeringUnits())
										  .arg(asp.highEngineeringUnits()));
						errorCodes[i] = ErrorCode::OutOfRange;
						continue;
					}

					osp.value = fValue;
					DEBUG_LOG_MSG(m_simLogger, tr("Override signal: id = '%1', fValue = '%2'").arg(asp.appSignalId()).arg(fValue));
				}
				else
				{
					int iValue = request.values[i].iValue;
					if (iValue < asp.lowEngineeringUnits() || iValue > asp.highEngineeringUnits())
					{
						DEBUG_LOG_MSG(m_simLogger,
									  tr("Override signal: id = '%1', iValue = '%2' is out of range (%3..%4)")
										  .arg(asp.appSignalId())
										  .arg(iValue)
										  .arg(asp.lowEngineeringUnits())
										  .arg(asp.highEngineeringUnits()));
						errorCodes[i] = ErrorCode::OutOfRange;
						continue;
					}

					osp.value = iValue;
					DEBUG_LOG_MSG(m_simLogger, tr("Override signal: id = '%1', iValue = '%2'").arg(asp.appSignalId()).arg(iValue));
				}
			}
		}

		overrideSignals.push_back(osp);

		auto result = m_client->OverrideSignals(overrideSignals);
		if (result.has_value() == false)
		{
			errorCodes[i] = ErrorCode::CannotWrite;
		}
		else
		{
			errorCodes[i] = ErrorCode::Success;
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

	switch (command)
	{
	case SGW_COMMAND_START:
	case SGW_COMMAND_RESUME:
		{
			auto status = m_client->CommandStart();
			if (status.has_value() == false)
			{
				DEBUG_LOG_ERR(m_appLogger, tr("CommandStart error: %1").arg(QString::fromUtf8(status.error().toStdString())));
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
			// Get all overrides
			//
			auto overridenSignals = m_client->GetOverriddenSignals();
			if (overridenSignals.has_value() == false)
			{
				DEBUG_LOG_ERR(m_appLogger,
							  tr("GetOverriddenSignals error: %1").arg(QString::fromUtf8(overridenSignals.error().toStdString())));
				reply = {RvUdpSim::ErrorCode::NoConnection, RvUdpSim::SimulatorStateCode::Unavailable};
			}
			else
			{
				// Remove all overrides
				//
				auto removeOverrides = m_client->RemoveOverrideSignals(overridenSignals.value());
				if (removeOverrides.has_value() == false)
				{
					DEBUG_LOG_ERR(m_appLogger,
								  tr("RemoveOverrideSignals error: %1").arg(QString::fromUtf8(removeOverrides.error().toStdString())));
					reply = {RvUdpSim::ErrorCode::NoConnection, RvUdpSim::SimulatorStateCode::Unavailable};
				}
			}

			// Stop the simulator
			//
			auto status = m_client->CommandStop();
			if (status.has_value() == false)
			{
				DEBUG_LOG_ERR(m_appLogger, tr("CommandStop error: %1").arg(QString::fromUtf8(status.error().toStdString())));
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
				DEBUG_LOG_ERR(m_appLogger, tr("CommandPause error: %1").arg(QString::fromUtf8(status.error().toStdString())));
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