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
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveRequestsServer thread started (connected %1)").
								arg(peerAddr().addressPortStr()));

	m_getNextReply = std::make_shared<Network::GetAppSignalStatesFromArchiveNextReply>();

	::google::protobuf::RepeatedPtrField<::Proto::AppSignalState>* states = m_getNextReply->mutable_appsignalstates();

	if (states != nullptr)
	{
		// allocate memory for states (slow operation)
		//
		states->Reserve(ARCH_REQUEST_MAX_STATES);

		for(int i = 0; i < ARCH_REQUEST_MAX_STATES; i++)
		{
			states->Add();
		}
	}
	else
	{
		assert(false);
	}
}

void TcpArchRequestsServer::onServerThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveRequestsServer thread finished  (connected sw %1, %2)").
								arg(connectedSoftwareInfo().equipmentID()).
								arg(peerAddr().addressPortStr()));

	finalizeArchRequest();
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

	if (m_archRequest != nullptr)
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

	// check signal hashes
	//
	QVector<Hash> signalHashes;
	QHash<Hash, bool> signalHashesMap;

	for(int i = 0; i < requestSignalsCount; i++)
	{
		Hash signalHash = request.signalhashes(i);

		if (m_archive->isSignalExists(signalHash) == true &&
			signalHashesMap.contains(signalHash) == false)
		{
			signalHashes.append(signalHash);
			signalHashesMap.insert(signalHash, true);
		}
	}

	if (signalHashes.count() == 0)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::NoSignals));
		sendReply(reply);
		return;
	}

	//

	m_archRequest = m_archive->startNewRequest(static_cast<E::TimeType>(request.timetype()),
														 request.starttime(),
														 request.endtime(),
														 signalHashes,
											   m_getNextReply);
	if (m_archRequest == nullptr)
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::RequestStartError));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));
	reply.set_requestid(m_archRequest->requestID());

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

	quint32 requestID = m_archRequest->requestID();

	if (m_archRequest == nullptr || requestID != request.requestid())
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::UnknownArchRequestID));
		reply.set_requestid(requestID);
		sendReply(reply);
		return;
	}

	if (m_archRequest->isDataReady() == false)
	{
		reply.set_error(static_cast<int>(NetworkError::Success));
		reply.set_archerror(static_cast<int>(ArchiveError::Success));
		reply.set_requestid(requestID);
		reply.set_dataready(false);
		sendReply(reply);

		qDebug() << "Data not ready";
		return;
	}

	Network::GetAppSignalStatesFromArchiveNextReply& nextReply = *m_getNextReply.get();

	sendReply(nextReply);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: send next reply, states = %2 (elapsed %3)").
				  arg(requestID).
				  arg(nextReply.statesinpartcount()).
				  arg(m_archRequest->timeElapsed()));

	if (nextReply.islastpart() == false)
	{
		m_archRequest->nextDataRequired();
	}
	else
	{
		finalizeArchRequest();
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

	if (m_archRequest == nullptr || m_archRequest->requestID() != request.requestid())
	{
		reply.set_error(static_cast<int>(NetworkError::ArchiveError));
		reply.set_archerror(static_cast<int>(ArchiveError::UnknownArchRequestID));
		sendReply(reply);
		return;
	}

	reply.set_error(static_cast<int>(NetworkError::Success));
	sendReply(reply);

	finalizeArchRequest();
}

void TcpArchRequestsServer::finalizeArchRequest()
{
	if (m_archRequest != nullptr)
	{
		quint32 requestID = m_archRequest->requestID();

		m_archRequest.reset();

		m_archive->finalizeRequest(requestID);
	}
}
