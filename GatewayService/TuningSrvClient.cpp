#include "TuningSrvClient.h"
#include "TuningGatewayServer.h"

// -------------------------------------------------------------------------------------
//
// TuningSrvClient class implementation
//
// -------------------------------------------------------------------------------------

TuningSrvClient::TuningSrvClient(std::shared_ptr<TgsSession>& session,
	const SoftwareInfo& softwareInfo,
	const HostAddressPort& serverAddressPort1,
	const HostAddressPort& serverAddressPort2,
	const QString& clientDescription,
	const QString& serverEquipmentID,
	const AppSignals& appSignals) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2,
				clientDescription, serverEquipmentID),
	m_session(session),
	m_appSignals(appSignals)
{
	Q_ASSERT(m_session != nullptr);
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
	m_session->connectedToTuningSrv = true;
	restartReceiveFile();
}

void TuningSrvClient::onDisconnection()
{
	m_session->connectedToTuningSrv = false;
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

void TuningSrvClient::getTuningSourcesState()
{
	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_requestQueue.pop_front();
		}

		m_requestQueue.push_back(Request{});

		Request& req = m_requestQueue.back();

		req.requestType = RequestType::SourceStates;
	}

	emit signal_sendNextRequest();
}

void TuningSrvClient::tuningSignalsRead(quint64 requestID, std::vector<Hash>& hashes)
{
	Q_ASSERT(requestID != 0);

	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_requestQueue.pop_front();
		}

		m_requestQueue.push_back(Request{});

		Request& req = m_requestQueue.back();

		req.requestType = RequestType::Read;
		req.rwRequestID = requestID;
		req.user.clear();
		req.apply = false;
		req.hashes.swap(hashes);
		req.values.clear();
	}

	emit signal_sendNextRequest();
}

void TuningSrvClient::tuningSignalsWrite(quint64 requestID,
										 const std::string& user,
										 bool apply,
										 std::vector<Hash>& hashes,
										 std::vector<double>& values)
{
	Q_ASSERT(requestID != 0);
	Q_ASSERT(hashes.size() == values.size());

	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_requestQueue.pop_front();
		}

		m_requestQueue.push_back(Request{});

		Request& req = m_requestQueue.back();

		req.requestType = RequestType::Write;
		req.rwRequestID = requestID;
		req.user = user;
		req.apply = apply;
		req.hashes.swap(hashes);
		req.values.swap(values);
	}

	emit signal_sendNextRequest();
}

void TuningSrvClient::tuningSignalsApply()
{
	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_requestQueue.pop_front();
		}

		m_requestQueue.push_back(Request{});

		Request& req = m_requestQueue.back();

		req.requestType = RequestType::Apply;
	}

	emit signal_sendNextRequest();
}

void TuningSrvClient::tuningChangeControlledSource(const std::string& moduleEquipmentID,
													bool activateControl)
{
	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.size() >= 3)
		{
			Q_ASSERT(false);
			m_requestQueue.pop_front();
		}

		m_requestQueue.push_back(Request{});

		Request& req = m_requestQueue.back();

		req.requestType = RequestType::ChangeConttrolledSource;
		req.moduleEquipmentID = moduleEquipmentID;
		req.activateControl = activateControl;
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

	case TDS_TUNING_SIGNALS_WRITE:
		onTuningSignalsWrite(replyData, replyDataSize);
		break;

	case TDS_TUNING_SIGNALS_APPLY:
		onTuningSignalsApply(replyData, replyDataSize);
		break;

	case TDS_CHANGE_CONTROLLED_TUNING_SOURCE:
		onChangeControlledSource(replyData, replyDataSize);
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
	if (m_activeRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::GetTuningSourcesStatesReply reply;

	reply.Clear();

	// check message integrity
	//
	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRequest.clear();
		return;
	}

	{
		std::lock_guard lg(m_session->condVarMutex);
		m_session->replyData.assign(replyData, replyData + replyDataSize);
	}

	m_session->condVar.notify_one();

	m_activeRequest.clear();
}

void TuningSrvClient::onTuningSignalsRead(const char* replyData, quint32 replyDataSize)
{
	if (m_activeRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::TuningSignalsReadReply reply;

	reply.Clear();

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRequest.clear();
		return;
	}

	if (m_activeRequest.rwRequestID != reply.readrequestid())
	{
		m_activeRequest.clear();
		return;
	}

	{
		std::lock_guard lg(m_session->condVarMutex);
		m_session->replyData.assign(replyData, replyData + replyDataSize);
	}

	m_session->condVar.notify_one();

	m_activeRequest.clear();
}

void TuningSrvClient::onTuningSignalsWrite(const char* replyData, quint32 replyDataSize)
{
	if (m_activeRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::TuningSignalsWriteReply reply;

	reply.Clear();

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRequest.clear();
		return;
	}

	if (m_activeRequest.rwRequestID != reply.writerequestid())
	{
		m_activeRequest.clear();
		return;
	}

	{
		std::lock_guard lg(m_session->condVarMutex);
		m_session->replyData.assign(replyData, replyData + replyDataSize);
	}

	m_session->condVar.notify_one();

	m_activeRequest.clear();
}

void TuningSrvClient::onTuningSignalsApply(const char* replyData, quint32 replyDataSize)
{
	if (m_activeRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::TuningSignalsApplyReply reply;

	reply.Clear();

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRequest.clear();
		return;
	}

	{
		std::lock_guard lg(m_session->condVarMutex);
		m_session->replyData.assign(replyData, replyData + replyDataSize);
	}

	m_session->condVar.notify_one();

	m_activeRequest.clear();
}

void TuningSrvClient::onChangeControlledSource(const char* replyData, quint32 replyDataSize)
{
	if (m_activeRequest.isNull())
	{
		Q_ASSERT(false);
		return;
	}

	thread_local Network::ChangeConrolledTuningSourceReply reply;

	reply.Clear();

	bool result = reply.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		m_activeRequest.clear();
		return;
	}

	{
		std::lock_guard lg(m_session->condVarMutex);
		m_session->replyData.assign(replyData, replyData + replyDataSize);
	}

	m_session->condVar.notify_one();

	m_activeRequest.clear();
}

void TuningSrvClient::sendNextRequest()
{
	if (isClearToSendRequest() == false || m_fileReady == false)
	{
		return;
	}

	{
		std::lock_guard lg(m_requestQueueMutex);

		if (m_requestQueue.empty() == false)
		{
			const Request& req = m_requestQueue.front();

			bool res = false;

			switch(req.requestType)
			{
			case RequestType::SourceStates:
				res = sendGetSourceStatesRequest(req);
				break;

			case RequestType::Read:
				res = sendReadSignalsRequest(req);
				break;

			case RequestType::Write:
				res = sendWriteSignalsRequest(req);
				break;

			case RequestType::Apply:
				res = sendApplySignalsRequest(req);
				break;

			case RequestType::ChangeConttrolledSource:
				res = sendChangeControlledSourceRequest(req);
				break;

			default:
				Q_ASSERT(false);
				res = true;
			}

			if (res == true)
			{
				m_requestQueue.pop_front();
			}

			return;
		}
	}
}

bool TuningSrvClient::sendGetSourceStatesRequest(const Request& req)
{
	Q_ASSERT(req.requestType == RequestType::SourceStates);

	setActiveRequest(req);

	Network::GetTuningSourcesStates request;
	sendRequest(TDS_GET_TUNING_SOURCES_STATES, request);

	return true;
}

bool TuningSrvClient::sendReadSignalsRequest(const Request& req)
{
	Q_ASSERT(req.requestType == RequestType::Read);

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

	setActiveRequest(req);

	return sendRequest(TDS_TUNING_SIGNALS_READ, readRequest);
}

bool TuningSrvClient::sendWriteSignalsRequest(const Request& req)
{
	Q_ASSERT(req.requestType == RequestType::Write);

	if (isClearToSendRequest() == false)
	{
		return false;
	}

	thread_local Network::TuningSignalsWrite writeRequest;

	writeRequest.Clear();

	writeRequest.set_writerequestid(req.rwRequestID);
	writeRequest.set_matsuser(req.user);
	writeRequest.set_autoapply(req.apply);

	size_t count = req.hashes.size();

	for(size_t i = 0; i < count; i++)
	{
		Hash h = req.hashes[i];

		const AppSignal* s = m_appSignals.getByHash(h);

		if (s == nullptr ||
			s->enableTuning() == false)
		{
			Q_ASSERT(false);
			continue;
		}

		TuningValue tunValue = TuningValue::createFromDouble(s->signalType(), s->analogSignalFormat(), req.values[i]);

		Network::TuningWriteCommand* wc = writeRequest.add_commands();

		wc->set_signalhash(h);
		tunValue.save(wc->mutable_value());
	}

	setActiveRequest(req);

	return sendRequest(TDS_TUNING_SIGNALS_WRITE, writeRequest);
}

bool TuningSrvClient::sendApplySignalsRequest(const Request& req)
{
	Q_ASSERT(req.requestType == RequestType::Apply);

	if (isClearToSendRequest() == false)
	{
		return false;
	}

	thread_local Network::TuningSignalsApply applyRequest;

	setActiveRequest(req);

	return sendRequest(TDS_TUNING_SIGNALS_APPLY, applyRequest);
}

bool TuningSrvClient::sendChangeControlledSourceRequest(const Request& req)
{
	Q_ASSERT(req.requestType == RequestType::ChangeConttrolledSource);

	if (isClearToSendRequest() == false)
	{
		return false;
	}

	thread_local Network::ChangeConrolledTuningSourceRequest request;

	request.Clear();

	request.set_takecontrol(true);
	request.set_tuningsourceequipmentid(req.moduleEquipmentID);
	request.set_activatecontrol(req.activateControl);

	setActiveRequest(req);

	return sendRequest(TDS_CHANGE_CONTROLLED_TUNING_SOURCE, request);
}

void TuningSrvClient::setActiveRequest(const Request& req)
{
	m_activeRequest.requestType = req.requestType;
	m_activeRequest.rwRequestID = req.rwRequestID;
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
											const QString& serverEquipmentID,
											const AppSignals& appSignals)
{
	m_client = new TuningSrvClient(session, softwareInfo,
								  serverAddressPort1, serverAddressPort2,
								  clientDescription, serverEquipmentID, appSignals);
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

void TuningSrvClientThread::getTuningSourcesState()
{
	m_client->getTuningSourcesState();
}

void TuningSrvClientThread::tuningSignalsRead(quint64 requestID, std::vector<Hash>& hashes)
{
	m_client->tuningSignalsRead(requestID, hashes);
}

void TuningSrvClientThread::tuningSignalsWrite(quint64 requestID,
	const std::string& user,
	bool apply,
	std::vector<Hash>& hashes,
	std::vector<double>& values)
{
	m_client->tuningSignalsWrite(requestID, user, apply, hashes, values);
}

void TuningSrvClientThread::tuningSignalsApply()
{
	m_client->tuningSignalsApply();
}

void TuningSrvClientThread::tuningChangeControlledSource(const std::string& moduleEquipmentID, bool activateControl)
{
	m_client->tuningChangeControlledSource(moduleEquipmentID, activateControl);
}

