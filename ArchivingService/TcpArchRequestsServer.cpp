#include "TcpArchRequestsServer.h"
#include "ArchRequest.h"

TcpArchRequestsServer::TcpArchRequestsServer(const SoftwareInfo& softwareInfo, Archive* archive, CircularLoggerShared logger) :
	Tcp::Server(softwareInfo),
	m_archive(archive),
	m_logger(logger)
{
	assert(m_archive != nullptr);
}

Tcp::Server* TcpArchRequestsServer::getNewInstance()
{
	return new TcpArchRequestsServer(localSoftwareInfo(), m_archive, m_logger);
}

void TcpArchRequestsServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	TEST_PTR_RETURN(m_archive);

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
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveRequestsServer thread started (connected sw %1, %2)").
								arg(connectedSoftwareInfo().equipmentID()).
								arg(peerAddr().addressPortStr()));
}

void TcpArchRequestsServer::onServerThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveRequestsServer thread finished  (connected sw %1, %2)").
								arg(connectedSoftwareInfo().equipmentID()).
								arg(peerAddr().addressPortStr()));

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
	assert(false);
	//

	if (m_request != nullptr)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::PreviousArchRequestIsNotFinished));
		sendReply(reply);
		return;
	}

	int requestSignalsCount = request.signalhashes_size();

	if (requestSignalsCount > ARCH_REQUEST_MAX_SIGNALS)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::ArchRequestSignalsExceed));
		sendReply(reply);
		return;
	}

	QVector<Hash> signalHashes;

	for(int i = 0; i < requestSignalsCount; i++)
	{
		Hash signalHash = request.signalhashes(i);

		if (m_archive->isSignalExists(signalHash) == true)
		{
			signalHashes.append(signalHash);
		}
	}

	if (signalHashes.count() == 0)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::NoSignals));
		sendReply(reply);
		return;
	}

	m_request = m_archive->createNewRequest(static_cast<E::TimeType>(request.timetype()),
														 request.starttime(),
														 request.endtime(),
														 signalHashes);

	if (m_request == nullptr)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::RequestStartError));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));
	reply.set_requestid(m_request->ID());

	sendReply(reply);
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

	if (m_currentRequestID == ArchRequestParam::NO_REQUEST || m_currentRequestID != request.requestid())
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
	return m_nextRequestNo.fetch_add(1);
}

TcpArchRequestsServerThread::TcpArchRequestsServerThread(const HostAddressPort& listenAddressPort,
			 Tcp::Server* server,
			 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{
}
