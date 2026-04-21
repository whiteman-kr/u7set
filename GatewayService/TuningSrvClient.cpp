#include "TuningSrvClient.h"
#include "TuningGatewayServer.h"

// -------------------------------------------------------------------------------------
//
// TuningSrvClient class implementation
//
// -------------------------------------------------------------------------------------

TuningSrvClient::TuningSrvClient(std::shared_ptr<TgsSession> session,
	const SoftwareInfo& softwareInfo,
	const HostAddressPort& serverAddressPort1,
	const HostAddressPort& serverAddressPort2,
	const QString& clientDescription,
	const QString& serverEquipmentID) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2,
				clientDescription, serverEquipmentID)
{
}

void TuningSrvClient::onClientThreadStarted()
{
	connect(this, &TuningSrvClient::signal_sendNextRequest, this, &TuningSrvClient::sendNextRequest, Qt::QueuedConnection);

	m_timer = new QTimer(this);

	connect(m_timer, &QTimer::timeout, this, &TuningSrvClient::onTimer);

	m_timerCtr = 0;
	m_timer->setInterval(TIMER_PERIOD);
	m_timer->start();
}

void TuningSrvClient::onClientThreadFinished()
{
}

void TuningSrvClient::onConnection()
{
	restartReceiveFile();
}

void TuningSrvClient::onDisconnection()
{
	clearReceiveFileVars();
}

bool TuningSrvClient::getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount)
{
	if (m_fileReady == false)
	{
		fileSize = 0;
		maxPartSize = 0;
		partCount = 0;
		return false;
	}

	fileSize = TO_QUINT64(m_tuningSourcesFileData.size());
	maxPartSize = TDS_TUNING_SOURCES_FILE_PART_SIZE;
	partCount = (fileSize + TDS_TUNING_SOURCES_FILE_PART_SIZE - 1) / TDS_TUNING_SOURCES_FILE_PART_SIZE;

	return true;
}

bool TuningSrvClient::getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize)
{
	if (m_fileReady == false)
	{
		return false;
	}

	quint64 fileSize = TO_UINT64(m_tuningSourcesFileData.size());

	quint64 partStart = partNo * TDS_TUNING_SOURCES_FILE_PART_SIZE;

	if (partStart >= fileSize)
	{
		return false;
	}

	partSize = std::min(fileSize - partStart, TDS_TUNING_SOURCES_FILE_PART_SIZE);

	fileData.insert(fileData.end(), m_tuningSourcesFileData.begin() + partStart,
					m_tuningSourcesFileData.begin() + partStart + partSize);
	return true;
}

bool TuningSrvClient::getTuningSourceStatesReply(std::vector<char>& reply)
{
	std::lock_guard lg(m_sourceStatesMutex);
	reply = m_sourceStatesReply;
	return true;
}

void TuningSrvClient::tuningSignalsRead(quint64 requestID,
	std::vector<Hash>& hashes,
	std::mutex* condVarMutex,
	std::condition_variable* condVar,
	std::vector<char>* replyData)
{
	TEST_PTR_RETURN(condVarMutex);
	TEST_PTR_RETURN(condVar);
	TEST_PTR_RETURN(replyData);

	{
		std::lock_guard lg(m_rwQueueMutex);

		if (m_rwRequestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_rwRequestQueue.pop_front();
		}

		m_rwRequestQueue.push_back(ReadWriteSignalsRequest{});

		ReadWriteSignalsRequest& req = m_rwRequestQueue.back();

		req.readRequest = true;
		req.rwRequestID = requestID;
		req.hashes.swap(hashes);
		req.values.clear();
		req.condVarMutex = condVarMutex;
		req.condVar = condVar;
		req.replyData = replyData;
	}

	emit signal_sendNextRequest();
}

void TuningSrvClient::onTimer()
{
	m_timerCtr += TIMER_PERIOD;
	sendNextRequest();
}

void TuningSrvClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case TDS_GET_TUNING_SOURCES_FILE:
		onGetNextFilePart(replyData, replyDataSize);
		break;

	case TDS_GET_TUNING_SOURCES_STATES:
		onGetTuningSourcesStates(replyData, replyDataSize);
		break;

	case TDS_TUNING_SIGNALS_READ:
		onTuningSignalsRead(replyData, replyDataSize);
		break;

	default:
		Q_ASSERT(false);
	}

	sendNextRequest();
}

void TuningSrvClient::onGetNextFilePart(const char* replyData, quint32 replyDataSize)
{
	Network::GetTuningSourcesFileReply reply;

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		restartReceiveFile();
		return;
	}

	quint64 partNo = reply.partno();

	if (partNo == 0)
	{
		m_tuningSourcesFileData.clear();
		m_tuningSourcesFileData.reserve(reply.filesize());
	}

	m_tuningSourcesFileData.insert(m_tuningSourcesFileData.end(),
								   reply.filepartdata().begin(),
								   reply.filepartdata().end());

	m_filePartNo++;

	if (m_filePartNo < reply.partscount())
	{
		requestNextFilePart();
		return;
	}

	quint64 crc64 = Crc::crc64(m_tuningSourcesFileData.data(),
							  TO_QINT64(m_tuningSourcesFileData.size()));

	if (reply.filecrc64() != crc64)
	{
		restartReceiveFile();
		return;
	}

	m_fileReady = true;
}

void TuningSrvClient::onGetTuningSourcesStates(const char* replyData, quint32 replyDataSize)
{
	thread_local Network::GetTuningSourcesStatesReply reply;
	thread_local int errCtr = 0;

	reply.Clear();

	// check message integrity
	//
	bool result = reply.ParseFromArray(replyData, replyDataSize);

	{
		std::lock_guard lg(m_sourceStatesMutex);

		if (result == true)
		{
			m_sourceStatesReply.assign(replyData, replyData + replyDataSize);
			errCtr = 0;
		}
		else
		{
			errCtr++;

			if (errCtr >= 3)
			{
				m_sourceStatesReply.clear();
				errCtr = 0;
			}
		}
	}
}

void TuningSrvClient::onTuningSignalsRead(const char* replyData, quint32 replyDataSize)
{
	if (m_activeRwRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::TuningSignalsReadReply reply;

	reply.Clear();

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRwRequest.clear();
		return;
	}

	if (m_activeRwRequest.rwRequestID != reply.readrequestid())
	{
		m_activeRwRequest.clear();
		return;
	}

	{
		std::lock_guard lg(*m_activeRwRequest.condVarMutex);
		m_activeRwRequest.replyData->assign(replyData, replyData + replyDataSize);
	}

	m_activeRwRequest.condVar->notify_one();

	m_activeRwRequest.clear();
}

void TuningSrvClient::sendNextRequest()
{
	if (isClearToSendRequest() == false || m_fileReady == false)
	{
		return;
	}

	{
		std::lock_guard lg(m_rwQueueMutex);

		if (m_rwRequestQueue.empty() == false)
		{
			const ReadWriteSignalsRequest& req = m_rwRequestQueue.front();

			bool res = false;

			if (req.readRequest == true)
			{
				res = sendReadSignalsRequest(req);
			}
			else
			{
				res = sendWriteSignalsRequest(req);
			}

			if (res == true)
			{
				m_rwRequestQueue.pop_front();
			}

			return;
		}
	}

	if (m_timerCtr >= REQUEST_SOURCE_STATES_PERIOD)
	{
		m_timerCtr = 0;
		Network::GetTuningSourcesStates request;
		sendRequest(TDS_GET_TUNING_SOURCES_STATES, request);
	}
}

bool TuningSrvClient::sendReadSignalsRequest(const ReadWriteSignalsRequest& req)
{
	Q_ASSERT(req.readRequest == true);

	if (isClearToSendRequest() == false)
	{
		return false;
	}

	thread_local Network::TuningSignalsRead readRequest;

	readRequest.Clear();

	readRequest.set_readrequestid(req.rwRequestID);

	for(const Hash h : req.hashes)
	{
		readRequest.add_signalhash(h);
	}

	setActiveRwRequest(req);

	return sendRequest(TDS_TUNING_SIGNALS_READ, readRequest);
}

bool TuningSrvClient::sendWriteSignalsRequest(const ReadWriteSignalsRequest& req)
{
	Q_ASSERT(false);
	return false;
}

void TuningSrvClient::setActiveRwRequest(const ReadWriteSignalsRequest& req)
{
	m_activeRwRequest.readRequest = req.readRequest;
	m_activeRwRequest.rwRequestID = req.rwRequestID;
	m_activeRwRequest.condVarMutex = req.condVarMutex;
	m_activeRwRequest.condVar = req.condVar;
	m_activeRwRequest.replyData = req.replyData;
}

void TuningSrvClient::restartReceiveFile()
{
	clearReceiveFileVars();
	requestNextFilePart();
}

void TuningSrvClient::requestNextFilePart()
{
	Network::GetTuningSourcesFileRequest request;

	request.set_partno(m_filePartNo);

	sendRequest(TDS_GET_TUNING_SOURCES_FILE, request);
}

void TuningSrvClient::clearReceiveFileVars()
{
	m_filePartNo = 0;
	m_filePartsCount = 0;
	m_fileReady = false;
	m_tuningSourcesFileData.clear();
}

// -------------------------------------------------------------------------------------
//
// TuningSrvClientThread class implementation
//
// -------------------------------------------------------------------------------------

TuningSrvClientThread::TuningSrvClientThread(std::shared_ptr<TgsSession> session,
											const SoftwareInfo& softwareInfo,
											const HostAddressPort& serverAddressPort1,
											const HostAddressPort& serverAddressPort2,
											const QString& clientDescription,
											const QString& serverEquipmentID)
{
	m_client = new TuningSrvClient(session, softwareInfo,
								  serverAddressPort1, serverAddressPort2,
								  clientDescription, serverEquipmentID);
	addWorker(m_client);
}

bool TuningSrvClientThread::getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount)
{
	return m_client->getTuningSourcesFileMetrics(fileSize, maxPartSize, partCount);
}

bool TuningSrvClientThread::getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize)
{
	return m_client->getTuningSourcesFilePart(partNo, fileData, partSize);
}

bool TuningSrvClientThread::getTuningSourceStatesReply(std::vector<char>& reply)
{
	return m_client->getTuningSourceStatesReply(reply);
}

void TuningSrvClientThread::tuningSignalsRead(quint64 requestID,
	std::vector<Hash>& hashes,
	std::mutex* condVarMutex,
	std::condition_variable* condVar,
	std::vector<char>* replyData)
{
	m_client->tuningSignalsRead(requestID, hashes, condVarMutex, condVar, replyData);
}


