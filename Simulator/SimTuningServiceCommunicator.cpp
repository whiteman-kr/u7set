#include "SimTuningServiceCommunicator.h"
#include "Simulator.h"

namespace Sim
{

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningServiceCommunicator class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningServiceCommunicator::TuningServiceCommunicator(Simulator* simulator,
														 const QString& tuningServiceEquipmentID) :
		m_simulator(simulator),
		m_tuningServiceEquipmentID(tuningServiceEquipmentID),
		m_log(m_simulator->log(), "TuningCommunicator")
	{
		Q_ASSERT(simulator);

		connect(m_simulator, &Simulator::projectUpdated, this, &TuningServiceCommunicator::projectUpdated);

		return;
	}

	TuningServiceCommunicator::~TuningServiceCommunicator()
	{
		stopProcessingThread();
	}

	bool TuningServiceCommunicator::startSimulation(QString profileName)
	{
		startProcessingThread(profileName);

		return true;
	}

	bool TuningServiceCommunicator::stopSimulation()
	{
		stopProcessingThread();

		return true;
	}

	Simulator* TuningServiceCommunicator::simulator() const
	{
		return m_simulator;
	}

	bool TuningServiceCommunicator::updateTuningRam(const QString& lmEquipmentId,
													const QString& portEquipmentId,
													const RamArea& ramArea,
													bool setSorChassisState,
													TimeStamp timeStamp)
	{
		if (softwareEnabled() == false)
		{
			return false;
		}

		if (m_processingThread != nullptr)
		{
			m_processingThread->updateTuningData(lmEquipmentId, portEquipmentId, ramArea, setSorChassisState, timeStamp);
		}

		return true;
	}

	void TuningServiceCommunicator::writeConfirmation(std::vector<qint64> confirmedRecords,
													  const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  bool setSorChassisState,
													  TimeStamp timeStamp)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->writeConfirmation(lmEquipmentId, portEquipmentId, confirmedRecords,
												  ramArea, setSorChassisState, timeStamp);
		}
	}

	void TuningServiceCommunicator::tuningModeEntered(const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  bool setSorChassisState,
													  TimeStamp timeStamp)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->tuningModeEntered(lmEquipmentId, portEquipmentId, ramArea, setSorChassisState, timeStamp);
		}
	}

	void TuningServiceCommunicator::tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->tuningModeLeft(lmEquipmentId, portEquipmentId);
		}
	}

	QString TuningServiceCommunicator::tuningServiceEquipmentID() const
	{
		return m_tuningServiceEquipmentID;
	}

	qint64 TuningServiceCommunicator::applyWrittenChanges(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		return writeTuningRecord(TuningRecord::createApplyChanges(lmEquipmentId, portEquipmentId));
	}

	qint64 TuningServiceCommunicator::writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, quint32 data, quint32 mask)
	{
		return writeTuningRecord(TuningRecord::createDword(lmEquipmentId, portEquipmentId, offsetW, data, mask));
	}

	qint64 TuningServiceCommunicator::writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, qint32 data)
	{
		return writeTuningRecord(TuningRecord::createSignedInt32(lmEquipmentId, portEquipmentId, offsetW, data));
	}

	qint64 TuningServiceCommunicator::writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, float data)
	{
		return writeTuningRecord(TuningRecord::createFloat(lmEquipmentId, portEquipmentId, offsetW, data));
	}

	std::queue<TuningRecord> TuningServiceCommunicator::fetchWriteTuningQueue(const QString& lmEquipmentId)
	{
		std::queue<TuningRecord> result;

		QMutexLocker l(&m_qmutex);

		auto node = m_writeTuningQueue.extract(lmEquipmentId);
		if (node.empty() == false)
		{
			result = std::move(node.mapped());
		}

		return result;
	}

	qint64 TuningServiceCommunicator::writeTuningRecord(TuningRecord&& r)
	{
		QMutexLocker l(&m_qmutex);

		m_writeTuningQueue[r.lmEquipmentId].push(std::move(r));

		return r.recordIndex;
	}

	void TuningServiceCommunicator::startProcessingThread(const QString& curProfileName)
	{
		Q_ASSERT(m_processingThread == nullptr);

		m_processingThread = new TuningRequestsProcessingThread(*this, curProfileName, m_log);
		m_processingThread->start();
	}

	void TuningServiceCommunicator::stopProcessingThread()
	{
		if (m_processingThread != nullptr)
		{
			bool res = m_processingThread->quitAndWait(2000);

			Q_ASSERT(res == true);

			delete m_processingThread;
			m_processingThread = nullptr;
		}
	}

	void TuningServiceCommunicator::projectUpdated()
	{
		// Project was loaded or cleared
		// Reset all queues here
		//
	}

	bool TuningServiceCommunicator::softwareEnabled() const
	{
		return m_simulator->software().enabled();
	}

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningRequestsProcessingThread class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningRequestsProcessingThread::TuningRequestsProcessingThread(TuningServiceCommunicator& tsCommunicator,
																   const QString& curProfileName,
																   ScopedLog& log) :
		m_tsCommunicator(tsCommunicator),
		m_curProfileName(curProfileName),
		m_sim(*tsCommunicator.simulator()),
		m_log(log)
	{
		std::shared_ptr<const TuningServiceSettings> settings =
				m_sim.software().getSettingsProfile<TuningServiceSettings>(m_tsCommunicator.tuningServiceEquipmentID(),
																			   curProfileName);

		if (settings == nullptr)
		{
			Q_ASSERT(false);
			m_log.writeError(QString("Settings profile '%1' is not found for AppDataService %2").
										arg(curProfileName).arg(m_tsCommunicator.tuningServiceEquipmentID()));
		}
		else
		{
			m_tuningRequestsReceivingIP = settings->tuningSimIP;
			m_tuningRepliesSendingIP = settings->tuningDataIP;

			initTuningSourcesHandlers(*settings.get());
		}
	}

	TuningRequestsProcessingThread::~TuningRequestsProcessingThread()
	{
	}

	void TuningRequestsProcessingThread::updateTuningData(const QString& lmEquipmentID,
														  const QString& portEquipmentID,
														  const RamArea& data,
														  bool setSorChassisState,
														  TimeStamp timeStamp)
	{
		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(lmEquipmentID, portEquipmentID);

		if (tsh != nullptr)
		{
			tsh->updateTuningData(data, setSorChassisState, timeStamp);
		}
	}

	void TuningRequestsProcessingThread::writeConfirmation(	const QString& lmEquipmentID,
															const QString& portEquipmentID,
															const std::vector<qint64>& confirmedRecords,
															const RamArea &ramArea,
															bool setSorChassisState,
															TimeStamp timeStamp)
	{
		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(lmEquipmentID, portEquipmentID);

		if (tsh != nullptr)
		{
			tsh->updateTuningData(ramArea, setSorChassisState, timeStamp);
		}

		m_queueMutex.lock();

		m_writeConfirmationQueue.emplace(lmEquipmentID, portEquipmentID, confirmedRecords);

		m_queueMutex.unlock();
	}


	void TuningRequestsProcessingThread::tuningModeEntered(const QString& lmEquipmentId,
														   const QString& portEquipmentId,
														   const RamArea& ramArea,
														   bool setSorChassisState,
														   TimeStamp timeStamp)
	{
		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(lmEquipmentId, portEquipmentId);

		if (tsh != nullptr)
		{
			tsh->tuningModeEntered(ramArea, setSorChassisState, timeStamp);
		}
	}

	void TuningRequestsProcessingThread::tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(lmEquipmentId, portEquipmentId);

		if (tsh != nullptr)
		{
			tsh->tuningModeLeft();
		}
	}

	void TuningRequestsProcessingThread::run()
	{
		m_log.writeMessage(QString("Tuning simulation is started (EquipmentID %1, receiving IP %2, sending IP %3)").
						arg(m_tsCommunicator.tuningServiceEquipmentID()).
						arg(m_tuningRequestsReceivingIP.addressPortStr()).
						arg(m_tuningRepliesSendingIP.addressPortStr()));

		m_thisThread = QThread::currentThread();

		while(isQuitRequested() == false)
		{
			if (m_tsCommunicator.softwareEnabled() == false)
			{
				msleep(10);
				continue;
			}

			bool result = tryCreateAndBindSocket();

			if (result == false)
			{
				continue;
			}

			receiveRequests();
		}

		closeSocket();

		m_log.writeMessage(QString("TuningRequestsProcessingThread is finished (EquipmentID %1)").
							arg(m_tsCommunicator.tuningServiceEquipmentID()));
	}

	void TuningRequestsProcessingThread::initTuningSourcesHandlers(const TuningServiceSettings& settings)
	{
		m_tuningSourcesByIP.clear();
		m_tuningSourcesByEquipmentID.clear();

		for(const TuningServiceSettings::TuningSource& ts : settings.sources)
		{
			std::shared_ptr<LogicModule> lm = m_sim.logicModule(ts.lmEquipmentID);

			if (lm == nullptr)
			{
				m_log.writeWarning(QString("Tuning source %1 isn't initialized").arg(ts.lmEquipmentID));
				continue;
			}

			auto tsh = std::make_shared<TuningSourceHandler>(m_tsCommunicator,
															ts.lmEquipmentID,
															ts.portEquipmentID,
															ts.tuningDataIP,
															lm->logicModuleExtraInfo());

			m_tuningSourcesByIP.insert({ts.tuningDataIP.address32(), tsh});
			m_tuningSourcesByEquipmentID.insert({{ts.lmEquipmentID, ts.portEquipmentID}, tsh});
		}
	}

	std::shared_ptr<TuningSourceHandler> TuningRequestsProcessingThread::getTuningSourceHandler(const QString& lmEquipmentID,
																								const QString& portEquipmentID)
	{
		auto p = m_tuningSourcesByEquipmentID.find({lmEquipmentID, portEquipmentID});

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			return std::shared_ptr<TuningSourceHandler>();
		}

		return p->second;
	}

	std::shared_ptr<TuningSourceHandler> TuningRequestsProcessingThread::getTuningSourceHandler(quint32 tuningSourceIP)
	{
		auto p = m_tuningSourcesByIP.find(tuningSourceIP);

		if (p == m_tuningSourcesByIP.end())
		{
			return std::shared_ptr<TuningSourceHandler>();
		}

		return p->second;
	}

	bool TuningRequestsProcessingThread::tryCreateAndBindSocket()
	{
		if (m_socket != nullptr)
		{
			closeSocket();
		}

		qint64 prevServerTime = -1;

		while(isQuitRequested() == false)
		{
			if (m_tsCommunicator.softwareEnabled() == false)
			{
				return false;
			}

			qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

			if (prevServerTime != -1 && serverTime - prevServerTime < 1000)
			{
				msleep(50);
				continue;
			}

			prevServerTime = serverTime;

			m_socket = new QUdpSocket();

			bool result = m_socket->bind(m_tuningRequestsReceivingIP.address(),
										 m_tuningRequestsReceivingIP.port());

			if (result == false)
			{
				m_log.writeWarning(QString("Tuning simulation listening socket binding error to %1").
								 arg(m_tuningRequestsReceivingIP.addressPortStr()));

				closeSocket();
				continue;
			}

			// bind Ok

			m_log.writeMessage(QString("Tuning simulation listening socket is created and bound to %1").
							   arg(m_tuningRequestsReceivingIP.addressPortStr()));

			QVariant newRecvBufSize(static_cast<int>(2 * 1024 * 1024));

			m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

			QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

#ifdef Q_DEBUG
			if (newRecvBufSize.toInt() != currentBufSize.toInt())
			{
				m_log.writeWarning(QString("Tuning simulation receive buffer size is not changed to required size."));
			}
#else
			Q_UNUSED(currentBufSize);
#endif
			break;
		}

		return m_socket != nullptr;
	}

	void TuningRequestsProcessingThread::closeSocket()
	{
		if (m_socket != nullptr)
		{
			m_socket->close();
			delete m_socket;
			m_socket = nullptr;
		}
	}

	void TuningRequestsProcessingThread::receiveRequests()
	{
		if (m_socket == nullptr)
		{
			return;
		}

		m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

		while(isQuitRequested() == false)
		{
			if (m_tsCommunicator.softwareEnabled() == false)
			{
				break;
			}

			bool result = true;

			result &= processWriteConfirmations();
			result &= processRequests();

			if (result == false)
			{
				break;
			}
		}

		cancelTuningSourceHandlersOperations();
		closeSocket();
	}

	bool TuningRequestsProcessingThread::processWriteConfirmations()
	{
		int processedCount = 0;

		WriteConfirmation wc;

		SimRupFotipV2 reply;

		do
		{
			m_queueMutex.lock();

			if (m_writeConfirmationQueue.empty() == true)
			{
				m_queueMutex.unlock();
				break;
			}

			wc = m_writeConfirmationQueue.front();

			m_writeConfirmationQueue.pop();

			m_queueMutex.unlock();

			processedCount++;

			std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(wc.lmEquipmentID, wc.portEquipmentID);

			if (tsh == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			bool sendReply = tsh->writeConfirmation(wc.confirmedRecordsIDs, &reply.rupFotipV2);

			if (sendReply == true)
			{
				finalizeAndSendReply(tsh->tuningSourceIP(), reply);
			}
		}
		while(processedCount < 100);

		return true;
	}

	bool TuningRequestsProcessingThread::processRequests()
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		QHostAddress from;

		qint64 size = m_socket->readDatagram(reinterpret_cast<char*>(&m_request),
											 sizeof(m_request),
											 &from);
		if (size == -1)
		{
			if (serverTime - m_lastRequestTime > 3000)
			{
				// recreate socket if has no requests in 3 seconds
				//
				closeSocket();
				return false;
			}

			msleep(1);
			return true;
		}

		m_lastRequestTime = serverTime;

		if (size != sizeof(SimRupFotipV2))
		{
			Q_ASSERT(false);
			return true;
		}

		if (m_request.rupFotipV2.checkCRC64() == false)
		{
			Q_ASSERT(false);
			return true;
		}

		quint16 simVersion = reverseUint16(m_request.simVersion);

		if (simVersion != 1)
		{
			Q_ASSERT(false);
			return true;
		}

		quint32 tuningSourceIP = reverseUint32(m_request.tuningSourceIP);

		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(tuningSourceIP);

		if (tsh == nullptr)
		{
			return true;
		}

		bool result = tsh->processRequest(m_request.rupFotipV2, &m_reply.rupFotipV2);

		if (result == false)
		{
			return true;				// reply will not be send
		}

		finalizeAndSendReply(tuningSourceIP, m_reply);

		return true;
	}

	void TuningRequestsProcessingThread::finalizeAndSendReply(quint32 tuningSourceIP, SimRupFotipV2& reply)
	{
		reply.rupFotipV2.rupHeader.reverseBytes();
		reply.rupFotipV2.fotipFrame.header.reverseBytes();
		reply.rupFotipV2.calcCRC64();

		reply.simVersion = reverseUint16(1);
		reply.tuningSourceIP = reverseUint32(tuningSourceIP);

		m_socket->writeDatagram(reinterpret_cast<const char*>(&reply),
								sizeof(reply),
								m_tuningRepliesSendingIP.address(),
								m_tuningRepliesSendingIP.port());
	}

	void TuningRequestsProcessingThread::cancelTuningSourceHandlersOperations()
	{
		std::queue<WriteConfirmation> emptyQueue;

		m_queueMutex.lock();
		m_writeConfirmationQueue.swap(emptyQueue);
		m_queueMutex.unlock();

		for(auto p : m_tuningSourcesByIP)
		{
			TEST_PTR_CONTINUE(p.second);

			p.second->cancelOperations();
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningSourceInterface class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningSourceHandler::TuningSourceHandler(TuningServiceCommunicator& tsCommunicator,
												 const QString& lmEquipmentID,
												 const QString& portEquipmentID,
												 const HostAddressPort& ip,
												 const ::LogicModuleInfo& logicModuleInfo) :
		m_tsCommunicator(tsCommunicator),
		m_lmEquipmentID(lmEquipmentID),
		m_portEquipmentID(portEquipmentID),
		m_tuningSourceIP(ip),
		m_moduleType(logicModuleInfo.moduleType()),
		m_lmNumber(logicModuleInfo.lmNumber),
		m_subsystemKey(logicModuleInfo.subsystemKey),
		m_lmUniqueID(logicModuleInfo.lmUniqueID)
	{
		std::shared_ptr<LogicModule> lm = m_tsCommunicator.simulator()->logicModule(m_lmEquipmentID);

		TEST_PTR_RETURN(lm);

		const LmDescription& lmDescription = lm->lmDescription();

		m_tuningFlashSizeB = lmDescription.flashMemory().m_tuningFrameCount * lmDescription.flashMemory().m_tuningFrameSize;
		m_tuningFlashFramePayloadB = lmDescription.flashMemory().m_tuningFramePayload;

		m_tuningDataStartAddrW = lmDescription.memory().m_tuningDataOffset;

		m_tuningDataSizeW = lmDescription.memory().m_tuningDataSize;
		m_tuningDataSizeB = m_tuningDataSizeW * WORD_SIZE_IN_BYTES;

		m_tuningDataFrameSizeW = lmDescription.memory().m_tuningDataFrameSize;
		m_tuningDataFramePayloadW = lmDescription.memory().m_tuningDataFramePayload;
		m_tuningDataFramePayloadB = m_tuningDataFramePayloadW * WORD_SIZE_IN_BYTES;

		Q_ASSERT(m_tuningDataFramePayloadB == FotipV2::TX_RX_DATA_SIZE);

		//

		m_tuningData = std::make_shared<RamArea>(	E::LogicModuleRamAccess::ReadWrite,
													m_tuningDataStartAddrW,
													m_tuningDataSizeW,
													false /* clearOnStartCycle */,
													QString("TuningData::") + m_lmEquipmentID);

		m_tuningDataReadBuffer.resize(m_tuningDataFramePayloadB);
	}

	TuningSourceHandler::~TuningSourceHandler()
	{
	}

	void TuningSourceHandler::updateTuningData(const RamArea& data, bool setSorChassisState, TimeStamp timeStamp)
	{
		Q_UNUSED(timeStamp);

		m_tuningDataMutex.lock();

		*m_tuningData.get() = data;

		m_tuningDataMutex.unlock();

		m_setSorChassisState.store(setSorChassisState);
	}

	bool TuningSourceHandler::writeConfirmation(const std::vector<qint64>& confirmationIDs, RupFotipV2* reply)
	{
		TEST_PTR_RETURN_FALSE(reply);

		if (m_waitingConfirmationID.has_value() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(confirmationIDs.size() == 1);
		Q_ASSERT(sizeof(*reply) == sizeof(m_delayedReply));

		bool result = false;

		for(qint64 confirmationID : confirmationIDs)
		{
			if (confirmationID != m_waitingConfirmationID)
			{
				continue;
			}

			m_waitingConfirmationID.reset();

			// read actual tuning data into reply
			//
			m_delayedReply.fotipFrame.header.flags.setSOR = m_setSorChassisState == true ? 1 : 0;

			readFrameData(m_delayedReply.fotipFrame.header.startAddressW,
						  &m_delayedReply.fotipFrame);

			switch(static_cast<FotipV2::OpCode>(m_delayedReply.fotipFrame.header.operationCode))
			{
			case FotipV2::OpCode::Write:

				m_delayedReply.fotipFrame.header.flags.successfulWrite = 1;
				result = true;

				//qDebug() << "Write confirmation " << confirmationID;

				break;

			case FotipV2::OpCode::Apply:

				m_delayedReply.fotipFrame.header.flags.succesfulApply = 1;
				result = true;

				//qDebug() << "Apply confirmation " << confirmationID;

				break;

			default:
				Q_ASSERT(false);
				result = false;
			}

			if (result == true)
			{
				memcpy(reply, &m_delayedReply, sizeof(m_delayedReply));
			}

			break;
		}

		return result;
	}

	void TuningSourceHandler::tuningModeEntered(const RamArea& ramArea, bool setSorChassisState, TimeStamp timeStamp)
	{
		updateTuningData(ramArea, setSorChassisState, timeStamp);

		m_tuningEnabled.store(true);
	}

	void TuningSourceHandler::tuningModeLeft()
	{
		m_tuningEnabled.store(false);

		cancelOperations();
	}

	bool TuningSourceHandler::processRequest(const RupFotipV2& request, RupFotipV2* reply)
	{
		if (m_tuningEnabled == false)
		{
			return false;			// no send replies while tuning disabled
		}

		if (m_waitingConfirmationID.has_value() == true)
		{
			Q_ASSERT(false);
			cancelOperations();
		}

		Rup::Header requestRupHeader = request.rupHeader;

		requestRupHeader.reverseBytes();

		if (checkRequestRupHeader(requestRupHeader) == false)
		{
			// reply will not be send if Rup::Header errors were detected
			//
			return false;
		}

		FotipV2::Header requestFotipHeader = request.fotipFrame.header;

		requestFotipHeader.reverseBytes();

		FotipV2::HeaderFlags replyFlags;

		replyFlags.all = 0;

		bool requestFotipHeaderOK = checkRequestFotipHeader(requestFotipHeader, &replyFlags);

		// reply Rup::Header initialization
		//
		Rup::Header& replyRupHeader = reply->rupHeader;

		replyRupHeader.protocolVersion = Rup::VERSION;
		replyRupHeader.numerator = requestRupHeader.numerator;
		replyRupHeader.frameSize = Socket::ENTIRE_UDP_SIZE;

		replyRupHeader.flags.all = 0;
		replyRupHeader.flags.tuningData = 1;
		replyRupHeader.moduleType = m_moduleType;
		replyRupHeader.framesQuantity = 1;
		replyRupHeader.frameNumber = 0;

		// reply FotipV2::Header initialization
		//
		FotipV2::Header& replyFotipHeader = reply->fotipFrame.header;

		replyFotipHeader.protocolVersion = FotipV2::VERSION;
		replyFotipHeader.uniqueId = m_lmUniqueID;
		replyFotipHeader.subsystemKey.lmNumber = m_lmNumber;
		replyFotipHeader.subsystemKey.subsystemCode = m_subsystemKey;
		replyFotipHeader.operationCode = requestFotipHeader.operationCode;
		replyFotipHeader.dataType = requestFotipHeader.dataType;
		replyFotipHeader.fotipFrameSizeB = sizeof(FotipV2::Frame);
		replyFotipHeader.romSizeB = m_tuningFlashSizeB;
		replyFotipHeader.romFrameSizeB = m_tuningFlashFramePayloadB;

		replyFotipHeader.startAddressW = requestFotipHeader.startAddressW;
		replyFotipHeader.offsetInFrameW = requestFotipHeader.offsetInFrameW;

		replyFlags.setSOR = m_setSorChassisState == true ? 1 : 0;

		replyFotipHeader.flags = replyFlags;

		memset(replyFotipHeader.reserv, 0, sizeof(replyFotipHeader.reserv));

		reply->fotipFrame.analogCmpErrors.all = 0;

		memset(reply->fotipFrame.data, 0, sizeof(reply->fotipFrame.data));
		memset(reply->fotipFrame.reserv, 0, sizeof(reply->fotipFrame.reserv));

		if (requestFotipHeaderOK == false)
		{
			return true;			// send immediately reply with error code in replyFotipHeader.flags
		}

		bool sendReplyImmediately = true;

		switch(static_cast<FotipV2::OpCode>(requestFotipHeader.operationCode))
		{
		case FotipV2::OpCode::Read:
			processReadRequest(request.fotipFrame, &reply->fotipFrame, &sendReplyImmediately);
			break;

		case FotipV2::OpCode::Write:
			processWriteRequest(request.fotipFrame, &reply->fotipFrame, &sendReplyImmediately);
			break;

		case FotipV2::OpCode::Apply:
			processApplyRequest(&sendReplyImmediately);
			break;

		default:
			Q_ASSERT(false);
		}

		if (sendReplyImmediately == false)
		{
			memcpy(&m_delayedReply, reply, sizeof(m_delayedReply));
			return false;
		}

		return true;
	}

	void TuningSourceHandler::cancelOperations()
	{
		m_waitingConfirmationID.reset();
	}

	bool TuningSourceHandler::checkRequestRupHeader(const Rup::Header& rupHeader)
	{
		if (rupHeader.protocolVersion != Rup::VERSION)
		{
			return false;
		}

		if (rupHeader.frameSize != Socket::ENTIRE_UDP_SIZE)
		{
			return false;
		}

		if ((rupHeader.flags.tuningData == 1 &&
			rupHeader.flags.appData == 0 &&
			rupHeader.flags.diagData == 0 &&
			rupHeader.flags.test == 0) == false)
		{
			return false;
		}

		if (rupHeader.moduleType != m_moduleType)
		{
			//	due to "DisableModulesTypeChecking" property of TuningService
			//	this check mey be isn't critical
		}

		if (rupHeader.framesQuantity != 1)
		{
			return false;
		}

		if (rupHeader.frameNumber != 0)
		{
			return false;
		}

		return true;
	}

	bool TuningSourceHandler::checkRequestFotipHeader(const FotipV2::Header& requestFotipHeader, FotipV2::HeaderFlags* replyFlags)
	{
		if (requestFotipHeader.protocolVersion != FotipV2::VERSION)
		{
			replyFlags->versionError = 1;
		}

		if (requestFotipHeader.uniqueId != m_lmUniqueID)
		{
			replyFlags->idError = 1;
		}

		if (requestFotipHeader.subsystemKey.lmNumber != m_lmNumber ||
			requestFotipHeader.subsystemKey.subsystemCode != m_subsystemKey)
		{
			replyFlags->subsystemKeyError = 1;
		}

		if (requestFotipHeader.fotipFrameSizeB != sizeof(FotipV2::Frame))
		{
			replyFlags->frameSizeError = 1;
		}

		if (requestFotipHeader.romSizeB !=  m_tuningFlashSizeB)
		{
			replyFlags->romSizeError = 1;
		}

		if (requestFotipHeader.romFrameSizeB != m_tuningFlashFramePayloadB)
		{
			replyFlags->romFrameSizeError = 1;
		}

		switch(static_cast<FotipV2::OpCode>(requestFotipHeader.operationCode))
		{
		case FotipV2::OpCode::Read:
		case FotipV2::OpCode::Write:
		case FotipV2::OpCode::Apply:
			break;

		default:
			replyFlags->operationCodeError = 1;
		}

		switch(static_cast<FotipV2::DataType>(requestFotipHeader.dataType))
		{
		case FotipV2::DataType::AnalogSignedInt:
		case FotipV2::DataType::AnalogFloat:
		case FotipV2::DataType::Discrete:
			break;

		default:
			replyFlags->dataTypeError = 1;
		};

		if (requestFotipHeader.startAddressW < m_tuningDataStartAddrW ||
			requestFotipHeader.startAddressW > (m_tuningDataStartAddrW + m_tuningDataSizeW) ||
			((requestFotipHeader.startAddressW - m_tuningDataStartAddrW) % m_tuningDataFramePayloadW) != 0)
		{
			replyFlags->startAddressError = 1;
		}

		if (requestFotipHeader.offsetInFrameW >= m_tuningDataFramePayloadW ||	// "equal" in condition is Ok!
			(requestFotipHeader.offsetInFrameW % 2) != 0 )						// possible offsetInFrameW values - even in range 0..507 only
		{
			replyFlags->offsetError = 1;
		}

		return replyFlags->all == 0;
	}

	void TuningSourceHandler::processReadRequest(const FotipV2::Frame& request,
												 FotipV2::Frame* reply,
												 bool* sendReplyImmediately)
	{
		TEST_PTR_RETURN(reply);
		TEST_PTR_RETURN(sendReplyImmediately);

		quint32 requestedTuningDataStartAddrW = reverseUint32(request.header.startAddressW);

		readFrameData(requestedTuningDataStartAddrW, reply);

		*sendReplyImmediately = true;
	}

	void TuningSourceHandler::processWriteRequest(const FotipV2::Frame& request,
												  FotipV2::Frame* reply,
												  bool* sendReplyImmediately)
	{
		TEST_PTR_RETURN(reply);
		TEST_PTR_RETURN(sendReplyImmediately);

		quint32 frameStartAddrW = reverseUint32(request.header.startAddressW);
		quint32 offsetInFrameW = reverseUint32(request.header.offsetInFrameW);

		quint32 writeAddrW = frameStartAddrW + offsetInFrameW;

		FotipV2::HeaderFlags& replyFlags = reply->header.flags;

		switch(static_cast<FotipV2::DataType>(reverseUint16(request.header.dataType)))
		{
		case FotipV2::DataType::AnalogSignedInt:
			{
				qint32 value = reverseInt32(request.write.analogSignedIntValue);

				qint32 lowBound = 0;

				m_tuningData->readSignedInt(frameStartAddrW + m_tuningDataFramePayloadW + offsetInFrameW,
											&lowBound, E::ByteOrder::BigEndian, false);
				qint32 highBound = 0;

				m_tuningData->readSignedInt(frameStartAddrW + m_tuningDataFramePayloadW * 2 + offsetInFrameW,
											&highBound, E::ByteOrder::BigEndian, false);

				if (value >= lowBound && value <= highBound)
				{
					replyFlags.successfulCheck = 1;

					m_waitingConfirmationID = m_tsCommunicator.writeTuningSignedInt32(m_lmEquipmentID,
																					   m_portEquipmentID,
																					   writeAddrW,
																					   value);
					*sendReplyImmediately = false;
				}
				else
				{
					if (value < lowBound)
					{
						reply->analogCmpErrors.lowBoundCheckError = 1;
					}
					else
					{
						if (value > highBound)
						{
							reply->analogCmpErrors.highBoundCheckError = 1;
						}
					}

					readFrameData(frameStartAddrW, reply);

					replyFlags.successfulCheck = 0;
					replyFlags.successfulWrite = 0;

					*sendReplyImmediately = true;
				}
			}

			break;

		case FotipV2::DataType::AnalogFloat:
			{
				float value = reverseFloat(request.write.analogFloatValue);

				float lowBound = 0;

				m_tuningData->readFloat(frameStartAddrW + m_tuningDataFramePayloadW + offsetInFrameW,
											&lowBound, E::ByteOrder::BigEndian, false);
				float highBound = 0;

				m_tuningData->readFloat(frameStartAddrW + m_tuningDataFramePayloadW * 2 + offsetInFrameW,
											&highBound, E::ByteOrder::BigEndian, false);

				if (value >= lowBound && value <= highBound)
				{
					replyFlags.successfulCheck = 1;

					m_waitingConfirmationID = m_tsCommunicator.writeTuningFloat(m_lmEquipmentID,
																				 m_portEquipmentID,
																				 writeAddrW,
																				 value);
					*sendReplyImmediately = false;
				}
				else
				{
					if (value < lowBound)
					{
						reply->analogCmpErrors.lowBoundCheckError = 1;
					}
					else
					{
						if (value > highBound)
						{
							reply->analogCmpErrors.highBoundCheckError = 1;
						}
					}

					readFrameData(frameStartAddrW, reply);

					replyFlags.successfulCheck = 0;
					replyFlags.successfulWrite = 0;

					*sendReplyImmediately = true;
				}
			}

			break;

		case FotipV2::DataType::Discrete:
			{
				quint32 value = reverseUint32(request.write.discreteValue);
				quint32 mask = reverseUint32(request.write.bitMask);

				m_waitingConfirmationID = m_tsCommunicator.writeTuningDword(m_lmEquipmentID, m_portEquipmentID, writeAddrW, value, mask);

				replyFlags.successfulCheck = 1;

				*sendReplyImmediately = false;
			}

			break;

		default:
			Q_ASSERT(false);		// unknown data type should be detected early in checkRequestFotipHeader

			readFrameData(frameStartAddrW, reply);

			replyFlags.successfulWrite = 0;
			replyFlags.dataTypeError = 1;

			*sendReplyImmediately = true;
		}
	}

	void TuningSourceHandler::processApplyRequest(bool* sendReplyImmediately)
	{
		TEST_PTR_RETURN(sendReplyImmediately);

		m_waitingConfirmationID = m_tsCommunicator.applyWrittenChanges(m_lmEquipmentID, m_portEquipmentID);

		*sendReplyImmediately = false;
	}

	void TuningSourceHandler::readFrameData(quint32 startFrameAddrW, FotipV2::Frame* reply)
	{
		m_tuningDataMutex.lock();

		bool res = m_tuningData->readToBuffer<std::vector<quint8>>(startFrameAddrW,
																   m_tuningDataFramePayloadW,
																   &m_tuningDataReadBuffer,
																   false);
		Q_ASSERT(res == true);

		m_tuningDataMutex.unlock();

/*		if (startFrameAddrW == 46336 ||``
				startFrameAddrW == 46336 + 508 ||
				startFrameAddrW == 46336 + 508 + 508)
		{
			for(int i = 0; i < 3; i++)
			{
				float* ptr = reinterpret_cast<float*>(m_tuningDataReadBuffer.data()) + i;

				float value = reverseFloat(*ptr);

				qDebug() << C_STR(QString("addr: %1 value %2").arg(startFrameAddrW + i).arg(value));
			}
		}

		qDebug() << "\n";*/

		if (m_tuningDataReadBuffer.size() == FotipV2::TX_RX_DATA_SIZE)
		{
			memcpy(reply->data, m_tuningDataReadBuffer.data(), FotipV2::TX_RX_DATA_SIZE);
		}
		else
		{
			Q_ASSERT(false);
		}
	}

}

