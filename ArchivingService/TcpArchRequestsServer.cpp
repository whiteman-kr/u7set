#include "TcpArchRequestsServer.h"

QMutex TcpArchRequestsServer::m_requestNoMutex;
quint32 TcpArchRequestsServer::m_nextRequestNo = 1;

TcpArchRequestsServer::TcpArchRequestsServer(ArchRequestThread& archRequestThread, CircularLoggerShared logger) :
	m_archRequestThread(archRequestThread),
    m_logger(logger)
{
}

Tcp::Server* TcpArchRequestsServer::getNewInstance()
{
	return new TcpArchRequestsServer(m_archRequestThread, m_logger);
}

void TcpArchRequestsServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
    switch(requestID)
    {
	case ARCHS_GET_APP_SIGNALS_STATES_START:
		onGetSignalStatesFromArchiveStart(requestData, requestDataSize);
		break;

	case ARCHS_GET_APP_SIGNALS_STATES_NEXT:
		onGetSignalStatesFromArchiveNext(requestData, requestDataSize);
		break;

	case ARCHS_GET_APP_SIGNALS_STATES_CANCEL:
		onGetSignalStatesFromArchiveCancel(requestData, requestDataSize);
		break;

    default:
		assert(false);
    }
}

void TcpArchRequestsServer::onServerThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, "TcpArchiveRequestsServer thread started");
}

void TcpArchRequestsServer::onServerThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, "TcpArchiveRequestsServer thread finished");

	finalizeCurrentRequest();
}

void TcpArchRequestsServer::onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize)
{
	QTime startTime;

	startTime.start();

	Network::GetAppSignalStatesFromArchiveStartRequest request;
	Network::GetAppSignalStatesFromArchiveStartReply reply;

	bool result = request.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	if (result == false)
	{
		reply.set_error(static_cast<int>(NetworkError::ParseRequestError));
		sendReply(reply);
		return;
	}

	//
	// check request.clear_clientequipmentid() here!!!
	//

	if (m_currentRequest != nullptr)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::PreviousArchRequestIsNotFinished));
		sendReply(reply);
		return;
	}

	int requestSignalsCount = request.signalhashes_size();

	if (requestSignalsCount <= 0)
	{
		assert(false);
		return;
	}

	if (requestSignalsCount > ARCH_REQUEST_MAX_SIGNALS)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::ArchRequestSignalsExceed));
		sendReply(reply);
		return;
	}

	quint32 newRequestID = getNewRequestID();

	reply.set_error(static_cast<int>(NetworkError::Success));
	reply.set_requestid(newRequestID);

	sendReply(reply);

	// pass request to ArchRequestThread

	ArchRequestParam param;

	param.timeType = static_cast<E::TimeType>(request.timetype());

	param.startTime = request.starttime();
	param.endTime = request.endtime();

	param.signalHashesCount = requestSignalsCount;

	for(int i = 0; i < requestSignalsCount; i++)
	{
		param.signalHashes[i] = request.signalhashes(i);
	}

	param.requestID = newRequestID;

	m_currentRequest = m_archRequestThread.startNewRequest(param, startTime);
}

void TcpArchRequestsServer::onGetSignalStatesFromArchiveNext(const char* requestData, quint32 requestDataSize)
{
	Network::GetAppSignalStatesFromArchiveNextRequest request;

	bool result = request.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	Network::GetAppSignalStatesFromArchiveNextReply reply;

	if (result == false)
	{
		reply.set_error(static_cast<int>(NetworkError::ParseRequestError));
		sendReply(reply);
		return;
	}

	if (m_currentRequest == nullptr || m_currentRequest->requestID() != request.requestid())
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::UnknownArchRequestID));
		reply.set_requestid(m_currentRequest->requestID());
		sendReply(reply);
		return;
	}

	if (m_currentRequest->isDataReady() == false)
	{
		reply.set_error(static_cast<int>(NetworkError::Success));
		reply.set_archerror(static_cast<int>(ArchiveError::Success));
		reply.set_requestid(m_currentRequest->requestID());
		reply.set_dataready(false);
/*		DEBUG_LOG_MSG(m_logger, QString("RequestID %1: send next reply, no data ready (elapsed %2)").
					  arg(m_currentRequest->requestID()).
					  arg(m_currentRequest->timeElapsed()));*/
		sendReply(reply);
		return;
	}

	if (m_currentRequest->archError() != ArchiveError::Success)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(m_currentRequest->archError()));
		reply.set_requestid(m_currentRequest->requestID());
		sendReply(reply);

		m_archRequestThread.finalizeRequest(m_currentRequest->requestID());

		m_currentRequest.reset();

		return;
	}

	Network::GetAppSignalStatesFromArchiveNextReply& nextReply = m_currentRequest->getNextReply();

	sendReply(nextReply);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: send next reply, states = %2 (elapsed %3)").
				  arg(m_currentRequest->requestID()).
				  arg(nextReply.statesinpartcount()).
				  arg(m_currentRequest->timeElapsed()));

	if (nextReply.islastpart() == false)
	{
		m_archRequestThread.getNextData(m_currentRequest);
	}
	else
	{
		finalizeCurrentRequest();
	}
}

void TcpArchRequestsServer::onGetSignalStatesFromArchiveCancel(const char* requestData, quint32 requestDataSize)
{
	Network::GetAppSignalStatesFromArchiveCancelRequest request;
	Network::GetAppSignalStatesFromArchiveCancelReply reply;

	bool result = request.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	if (result == false)
	{
		reply.set_error(static_cast<int>(NetworkError::ParseRequestError));
		sendReply(reply);
		return;
	}

	if (m_currentRequest == nullptr || m_currentRequest->requestID() != request.requestid())
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::UnknownArchRequestID));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));
	sendReply(reply);

	finalizeCurrentRequest();
}

void TcpArchRequestsServer::finalizeCurrentRequest()
{
	if (m_currentRequest == nullptr)
	{
		return;
	}

	quint32 requestID = m_currentRequest->requestID();

	m_currentRequest.reset();

	m_archRequestThread.finalizeRequest(requestID);
}

quint32 TcpArchRequestsServer::getNewRequestID()
{
	m_requestNoMutex.lock();

	quint32 requestID = m_nextRequestNo;

	m_nextRequestNo++;

	if (m_nextRequestNo == 0)
	{
		m_nextRequestNo = 1;
	}

	m_requestNoMutex.unlock();

	return requestID;
}

TcpArchiveRequestsServerThread::TcpArchiveRequestsServerThread(const HostAddressPort& listenAddressPort,
			 Tcp::Server* server,
			 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{
}
