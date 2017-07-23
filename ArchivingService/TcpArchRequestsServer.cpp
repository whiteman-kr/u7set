#include "TcpArchRequestsServer.h"

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
		break;

	case ARCHS_GET_APP_SIGNALS_STATES_CANCEL:
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
}

void TcpArchRequestsServer::onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize)
{
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
		reply.set_error(static_cast<int>(NetworkError::PreviousArchRequestIsNotFinished));
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
		reply.set_error(static_cast<int>(NetworkError::ArchRequestSignalsExceed));
		sendReply(reply);
		return;
	}

	quint32 newRequestID = getNewRequestID();

	reply.set_error(static_cast<int>(NetworkError::Success));
	reply.set_requestid(newRequestID);

	sendReply(reply);

	// pass request to ArchRequestThread

	ArchRequestParam param;

	param.timeType = static_cast<TimeType>(request.timetype());

	param.startTime = request.starttime();
	param.endTime = request.endtime();

	param.hashesCount = requestSignalsCount;

	for(int i = 0; i < requestSignalsCount; i++)
	{
		param.signalHashes[i] = request.signalhashes(i);
	}

	param.requestID = newRequestID;

	param.archRequestServer = this;

	m_currentRequest = m_archRequestThread.startNewRequest(param);
}

void TcpArchRequestsServer::onGetSignalStatesFromArchiveNext(const char* requestData, quint32 requestDataSize)
{
	Network::GetAppSignalStatesFromArchiveNextRequest request;
	Network::GetAppSignalStatesFromArchiveNextReply reply;

	bool result = request.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	if (result == false)
	{
		reply.set_error(static_cast<int>(NetworkError::ParseRequestError));
		sendReply(reply);
		return;
	}

	if (m_currentRequest == nullptr || m_currentRequest->requestID() != request.requestid())
	{
		reply.set_error(static_cast<int>(NetworkError::UnknownArchRequestID));
		sendReply(reply);
		return;
	}

	if (m_currentRequest->isDataReady() == false)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchRequestInProgress));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));

	reply.set_requestid(m_currentRequest->requestID());

	reply.set_totalstatescount(m_currentRequest->totalStates());
	reply.set_sentstatescount(m_currentRequest->sentStates());

	reply.set_statesinpartcount(0);
	reply.set_islastpart(true);

	//repeated Proto.AppSignalState appSignalStates = 7;

	sendReply(reply);
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
		reply.set_error(static_cast<int>(NetworkError::UnknownArchRequestID));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));
	sendReply(reply);

	m_currentRequest.reset();

	m_archRequestThread.finalizeRequest(request.requestid());
}

quint32 TcpArchRequestsServer::getNewRequestID()
{
	quint32 requestID = 0;

	Qt::HANDLE threadHandle = QThread::currentThreadId();

	requestID = CRC32(requestID, reinterpret_cast<const char*>(&threadHandle), sizeof(threadHandle), false);
	requestID = CRC32(requestID, reinterpret_cast<const char*>(&m_nextRequestNo), sizeof(m_nextRequestNo), true);

	m_nextRequestNo++;

	return requestID;
}

TcpArchiveRequestsServerThread::TcpArchiveRequestsServerThread(const HostAddressPort& listenAddressPort,
			 Tcp::Server* server,
			 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{
}
