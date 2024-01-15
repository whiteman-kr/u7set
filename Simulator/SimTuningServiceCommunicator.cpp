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

		for(TuningRequestsProcessingThread* thread : m_processingThreads)
		{
			thread->updateTuningData(lmEquipmentId, portEquipmentId, ramArea, setSorChassisState, timeStamp);
		}

		return true;
	}

	void TuningServiceCommunicator::writeConfirmation(qint64 confirmedRecordID,
													  const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  bool setSorChassisState,
													  TimeStamp timeStamp)
	{
		for(TuningRequestsProcessingThread* thread : m_processingThreads)
		{
			thread->writeConfirmation(lmEquipmentId, portEquipmentId, confirmedRecordID,
												  ramArea, setSorChassisState, timeStamp);
		}
	}

	void TuningServiceCommunicator::tuningModeEntered(const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  bool setSorChassisState,
													  TimeStamp timeStamp)
	{
		for(TuningRequestsProcessingThread* thread : m_processingThreads)
		{
			thread->tuningModeEntered(lmEquipmentId, portEquipmentId, ramArea, setSorChassisState, timeStamp);
		}
	}

	void TuningServiceCommunicator::tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		for(TuningRequestsProcessingThread* thread : m_processingThreads)
		{
			thread->tuningModeLeft(lmEquipmentId, portEquipmentId);
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

	ScopedLog& TuningServiceCommunicator::log()
	{
		return m_log;
	}

	qint64 TuningServiceCommunicator::writeTuningRecord(TuningRecord&& r)
	{
		QMutexLocker l(&m_qmutex);

		m_writeTuningQueue[r.lmEquipmentId].push(std::move(r));

		return r.recordIndex;
	}

	void TuningServiceCommunicator::startProcessingThread(const QString& curProfileName)
	{
		Q_ASSERT(m_processingThreads.size() == 0);

		std::shared_ptr<const TuningServiceSettings> settings =
			simulator()->software().getSettingsProfile<TuningServiceSettings>(tuningServiceEquipmentID(),
																			   curProfileName);

		if (settings == nullptr)
		{
			Q_ASSERT(false);
			m_log.writeError(QString("Settings profile '%1' is not found for TuningService %2").
										arg(curProfileName).arg(tuningServiceEquipmentID()));

			return;
		}

		for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
		{
			if (settings->channelSettings[channel].enable == false)
			{
				continue;
			}

			TuningRequestsProcessingThread* thread = new TuningRequestsProcessingThread(*this, curProfileName, settings, channel, m_log);

			m_processingThreads.push_back(thread);

			thread->start();
		}
	}

	void TuningServiceCommunicator::stopProcessingThread()
	{
		for(TuningRequestsProcessingThread* thread : m_processingThreads)
		{
			bool res = thread->quitAndWait(2000);

			Q_ASSERT(res == true);

			delete thread;
		}

		m_processingThreads.clear();
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
	// FotipProcessingNumeratorsMap class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	void FotipProcessingNumeratorsMap::appendNumerator(const QString& lmEquipmentID)
	{
		AUTO_LOCK(m_mapMutex);

		if (m_fotipProcessingNumeratorsMap.contains(lmEquipmentID) == false)
		{
			m_fotipProcessingNumeratorsMap.insert({lmEquipmentID, 0});
		}
	}

	quint64 FotipProcessingNumeratorsMap::getNextFotipProcessingNumerator(const QString& lmEquipmentID)
	{
		AUTO_LOCK(m_mapMutex);

		auto it = m_fotipProcessingNumeratorsMap.find(lmEquipmentID);

		if (it == m_fotipProcessingNumeratorsMap.end())
		{
			Q_ASSERT(false);
			return 0;
		}

		quint64 numerator = ++it->second;

		return numerator;
	}

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningRequestsProcessingThread class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningRequestsProcessingThread::TuningRequestsProcessingThread(TuningServiceCommunicator& tsCommunicator,
																   const QString& curProfileName,
																   std::shared_ptr<const TuningServiceSettings> settings,
																   int channel,
																   ScopedLog& log) :
		m_tsCommunicator(tsCommunicator),
		m_curProfileName(curProfileName),
		m_channel(channel),
		m_sim(*tsCommunicator.simulator()),
		m_log(log)
	{
		if (settings == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		Q_ASSERT(m_channel >= 0 && m_channel < TuningServiceSettings::CHANNELS_COUNT);

		m_controllerEquipmentID = settings->channelSettings[m_channel].serviceControllerEquipmentID;
		m_tuningRequestsReceivingIP = settings->channelSettings[m_channel].tuningSimIP;
		m_tuningRepliesSendingIP = settings->channelSettings[m_channel].tuningDataIP;

		initTuningSourcesHandlers(*settings.get());

		//qDebug() << "TuningRequestsProcessingThread " << C_STR(tsCommunicator.tuningServiceEquipmentID()) << "channel" << channel+1;
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
															qint64 confirmedRecordID,
															const RamArea &ramArea,
															bool setSorChassisState,
															TimeStamp timeStamp)
	{
		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(lmEquipmentID, portEquipmentID);

		if (tsh != nullptr)
		{
			tsh->updateTuningData(ramArea, setSorChassisState, timeStamp);

			m_queueMutex.lock();

			m_writeConfirmationQueue.emplace(lmEquipmentID, portEquipmentID, confirmedRecordID);

			m_queueMutex.unlock();
		}
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
		m_log.writeDebug(QString("Tuning simulation is started (EquipmentID %1, receiving IP %2, sending IP %3)").
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

		m_log.writeDebug(QString("TuningRequestsProcessingThread finished (EquipmentID %1)").
							arg(m_tsCommunicator.tuningServiceEquipmentID()));
	}

	void TuningRequestsProcessingThread::initTuningSourcesHandlers(const TuningServiceSettings& settings)
	{
		Q_ASSERT(m_channel >= 0 && m_channel < TuningServiceSettings::CHANNELS_COUNT);

		m_tuningSourcesByIP.clear();
		m_tuningSourcesByEquipmentID.clear();

		for(const TuningServiceSettings::TuningSource& ts : settings.channelSettings[m_channel].sources)
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

			QVariant newRecvBufSize(static_cast<int>(2 * 1024 * 1024));

			m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

			QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

#ifdef QT_DEBUG
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

		SimRupFotip reply;

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

			bool sendReply = tsh->writeConfirmation(wc.confirmedRecordID, &reply.rupFotip);

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

		if (size != sizeof(SimRupFotip))
		{
			logWarningThinned(__LINE__, QString("Tuning request from %1 wrong lenght %2, expected %3. "
												"Check that TuningService is in Simulation mode.").
														arg(m_controllerEquipmentID).arg(size).arg(sizeof(SimRupFotip)));
			return;
		}

		if (m_request.rupFotip.checkCRC64() == false)
		{
			logWarningThinned(__LINE__, QString("Tuning request from %1 wrong CRC64.").
													arg(m_controllerEquipmentID));
			return true;
		}

		quint16 simVersion = reverseUint16(m_request.simVersion);

		const int EXPECTED_SIM_VERSION = 1;

		if (simVersion != EXPECTED_SIM_VERSION)
		{
			logWarningThinned(__LINE__, QString("Tuning request from %1 wrong version %2, expected %3!").
												arg(m_controllerEquipmentID).arg(simVersion).arg(EXPECTED_SIM_VERSION));
			return true;
		}

		quint32 tuningSourceIP = reverseUint32(m_request.tuningSourceIP);

		std::shared_ptr<TuningSourceHandler> tsh = getTuningSourceHandler(tuningSourceIP);

		if (tsh == nullptr)
		{
			return true;
		}

		bool result = tsh->processRequest(m_request.rupFotip, &m_reply.rupFotip);

		if (result == false)
		{
			return true;				// reply will not be send
		}

		finalizeAndSendReply(tuningSourceIP, m_reply);

		return true;
	}

	void TuningRequestsProcessingThread::finalizeAndSendReply(quint32 tuningSourceIP, SimRupFotip& reply)
	{
		reply.rupFotip.rupHeader.reverseBytes();
		reply.rupFotip.fotipFrame.header.reverseBytes();
		reply.rupFotip.calcCRC64();

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

		for(auto& p : m_tuningSourcesByIP)
		{
			TEST_PTR_CONTINUE(p.second);

			p.second->cancelOperations();
		}
	}

	void TuningRequestsProcessingThread::logWarningThinned(int codeLine, const QString& warning)
	{
		static std::map<int, int> wrnCtrMap;		// codeLine => wrnCounter

		auto it = wrnCtrMap.find(codeLine);

		if (it == wrnCtrMap.end())
		{
			auto [newIt, b] = wrnCtrMap.emplace(codeLine, 0);
			it = newIt;
		}

		if ((it->second % 100) == 0)
		{
			m_log.writeWarning(warning);
		}

		it->second++;
	}

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningSourceInterface class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	FotipProcessingNumeratorsMap TuningSourceHandler::m_processingNumeratorsMap;

	TuningSourceHandler::TuningSourceHandler(TuningServiceCommunicator& tsCommunicator,
												 const QString& lmEquipmentID,
												 const QString& portEquipmentID,
												 const HostAddressPort& ip,
												 const ::LogicModuleInfo& logicModuleInfo) :
		m_tsCommunicator(tsCommunicator),
		m_log(tsCommunicator.log()),
		m_lmEquipmentID(lmEquipmentID),
		m_portEquipmentID(portEquipmentID),
		m_tuningSourceIP(ip),
		m_moduleType(logicModuleInfo.moduleType()),
		m_rupVersion(logicModuleInfo.lanControllers.rupVersion()),
		m_fotipVersion(logicModuleInfo.lanControllers.fotipVersion()),
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

		Q_ASSERT(m_tuningDataFramePayloadB == Fotip::TX_RX_DATA_SIZE);

		//

		m_tuningData = std::make_shared<RamArea>(	E::LogicModuleRamAccess::ReadWrite,
													m_tuningDataStartAddrW,
													m_tuningDataSizeW,
													false /* clearOnStartCycle */,
													QString("TuningData::") + m_lmEquipmentID);

		m_tuningDataReadBuffer.resize(m_tuningDataFramePayloadB);

		m_processingNumeratorsMap.appendNumerator(m_lmEquipmentID);
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

	bool TuningSourceHandler::writeConfirmation(qint64 confirmationID, RupFotip* reply)
	{
		TEST_PTR_RETURN_FALSE(reply);

		if (m_waitingConfirmationID.has_value() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		if (confirmationID != m_waitingConfirmationID)
		{
			Q_ASSERT(false);
			return false;
		}

		Q_ASSERT(sizeof(*reply) == sizeof(m_delayedReply));

		bool result = false;

		m_waitingConfirmationID.reset();

		// read actual tuning data into reply
		//
		m_delayedReply.fotipFrame.header.flags.setSOR = m_setSorChassisState == true ? 1 : 0;

		readFrameData(m_delayedReply.fotipFrame.header.startAddressW,
					  &m_delayedReply.fotipFrame);

		switch(static_cast<Fotip::OpCode>(m_delayedReply.fotipFrame.header.operationCode))
		{
		case Fotip::OpCode::Write:

			m_delayedReply.fotipFrame.header.flags.successfulWrite = 1;
			result = true;

			//qDebug() << "Write confirmation " << confirmationID;

			break;

		case Fotip::OpCode::Apply:

			m_delayedReply.fotipFrame.header.flags.succesfulApply = 1;
			result = true;

			break;

		default:
			Q_ASSERT(false);
			result = false;
		}

		if (result == true)
		{
			memcpy(reply, &m_delayedReply, sizeof(m_delayedReply));
			setFotipProcessingNumerator(reply);
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

	bool TuningSourceHandler::processRequest(const RupFotip& request, RupFotip* reply)
	{
		if (m_tuningEnabled == false)
		{
			return false;			// no send replies while tuning disabled
		}

		if (m_waitingConfirmationID.has_value() == true)
		{
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

		Fotip::Header requestFotipHeader = request.fotipFrame.header;

		requestFotipHeader.reverseBytes();

		Fotip::HeaderFlags replyFlags;

		replyFlags.all = 0;

		bool requestFotipHeaderOK = checkRequestFotipHeader(requestFotipHeader, &replyFlags);

		// reply Rup::Header initialization
		//
		Rup::Header& replyRupHeader = reply->rupHeader;

		replyRupHeader.protocolVersion = static_cast<quint16>(m_rupVersion);
		replyRupHeader.numerator = requestRupHeader.numerator;
		replyRupHeader.frameSize = Rup::ENTIRE_UDP_SIZE;

		replyRupHeader.flags.all = 0;
		replyRupHeader.flags.tuningData = 1;
		replyRupHeader.moduleType = static_cast<quint16>(m_moduleType);
		replyRupHeader.framesQuantity = 1;
		replyRupHeader.frameNumber = 0;
		replyRupHeader.timeStamp.setDateTime(QDateTime::currentDateTime());

		// reply Fotip::Header initialization
		//
		Fotip::Header& replyFotipHeader = reply->fotipFrame.header;

		replyFotipHeader.protocolVersion = static_cast<quint16>(m_fotipVersion);
		replyFotipHeader.uniqueId = m_lmUniqueID;
		replyFotipHeader.subsystemKey.lmNumber = m_lmNumber;
		replyFotipHeader.subsystemKey.subsystemCode = m_subsystemKey;
		replyFotipHeader.operationCode = requestFotipHeader.operationCode;
		replyFotipHeader.dataType = requestFotipHeader.dataType;
		replyFotipHeader.fotipFrameSizeB = sizeof(Fotip::Frame);
		replyFotipHeader.romSizeB = m_tuningFlashSizeB;
		replyFotipHeader.romFrameSizeB = static_cast<quint16>(m_tuningFlashFramePayloadB);

		replyFotipHeader.startAddressW = requestFotipHeader.startAddressW;
		replyFotipHeader.offsetInFrameW = requestFotipHeader.offsetInFrameW;

		replyFotipHeader.fotipProcessingNumerator = 0;	// here is only initialization (from Fotip::V3)
														// real value will set after request processing
		if (m_fotipVersion >= Fotip::V3)
		{
			replyFotipHeader.requestNumerator = requestFotipHeader.requestNumerator;
		}
		else
		{
			replyFotipHeader.requestNumerator = 0;
		}

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

		switch(static_cast<Fotip::OpCode>(requestFotipHeader.operationCode))
		{
		case Fotip::OpCode::Read:
			processReadRequest(request.fotipFrame, &reply->fotipFrame, &sendReplyImmediately);
			break;

		case Fotip::OpCode::Write:
			processWriteRequest(request.fotipFrame, &reply->fotipFrame, &sendReplyImmediately);
			break;

		case Fotip::OpCode::Apply:
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
		else
		{
			setFotipProcessingNumerator(reply);
		}

		return true;
	}

	void TuningSourceHandler::cancelOperations()
	{
		m_waitingConfirmationID.reset();
	}

	bool TuningSourceHandler::checkRequestRupHeader(const Rup::Header& rupHeader)
	{
		if (rupHeader.protocolVersion != m_rupVersion)
		{
			m_log.writeError(QString("Wrong RUP protocol version on %1. Received version %2, expected version %3").
							 arg(m_portEquipmentID).arg(rupHeader.protocolVersion).arg(m_rupVersion));
			return false;
		}

		if (rupHeader.frameSize != Rup::ENTIRE_UDP_SIZE)
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

	bool TuningSourceHandler::checkRequestFotipHeader(const Fotip::Header& requestFotipHeader, Fotip::HeaderFlags* replyFlags)
	{
		if (requestFotipHeader.protocolVersion != m_fotipVersion)
		{
			m_log.writeError(QString("Wrong FOTIP protocol version on %1. Received version %2, expected version %3").
							 arg(m_portEquipmentID).arg(requestFotipHeader.protocolVersion).arg(m_fotipVersion));
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

		if (requestFotipHeader.fotipFrameSizeB != sizeof(Fotip::Frame))
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

		switch(static_cast<Fotip::OpCode>(requestFotipHeader.operationCode))
		{
		case Fotip::OpCode::Read:
		case Fotip::OpCode::Write:
		case Fotip::OpCode::Apply:
			break;

		default:
			replyFlags->operationCodeError = 1;
		}

		switch(static_cast<Fotip::DataType>(requestFotipHeader.dataType))
		{
		case Fotip::DataType::AnalogSignedInt:
		case Fotip::DataType::AnalogFloat:
		case Fotip::DataType::Discrete:
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

	void TuningSourceHandler::processReadRequest(const Fotip::Frame& request,
												 Fotip::Frame* reply,
												 bool* sendReplyImmediately)
	{
		TEST_PTR_RETURN(reply);
		TEST_PTR_RETURN(sendReplyImmediately);

		quint32 requestedTuningDataStartAddrW = reverseUint32(request.header.startAddressW);

		readFrameData(requestedTuningDataStartAddrW, reply);

		*sendReplyImmediately = true;
	}

	void TuningSourceHandler::processWriteRequest(const Fotip::Frame& request,
												  Fotip::Frame* reply,
												  bool* sendReplyImmediately)
	{
		TEST_PTR_RETURN(reply);
		TEST_PTR_RETURN(sendReplyImmediately);

		quint32 frameStartAddrW = reverseUint32(request.header.startAddressW);
		quint32 offsetInFrameW = reverseUint32(request.header.offsetInFrameW);

		quint32 writeAddrW = frameStartAddrW + offsetInFrameW;

		Fotip::HeaderFlags& replyFlags = reply->header.flags;

		switch(static_cast<Fotip::DataType>(reverseUint16(request.header.dataType)))
		{
		case Fotip::DataType::AnalogSignedInt:
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

		case Fotip::DataType::AnalogFloat:
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

		case Fotip::DataType::Discrete:
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

	void TuningSourceHandler::readFrameData(quint32 startFrameAddrW, Fotip::Frame* reply)
	{
		m_tuningDataMutex.lock();

		bool res = m_tuningData->readToBuffer<std::vector<quint8>>(startFrameAddrW,
																   m_tuningDataFramePayloadW,
																   &m_tuningDataReadBuffer,
																   false);
		Q_ASSERT(res == true);

		m_tuningDataMutex.unlock();

		if (m_tuningDataReadBuffer.size() == Fotip::TX_RX_DATA_SIZE)
		{
			memcpy(reply->data, m_tuningDataReadBuffer.data(), Fotip::TX_RX_DATA_SIZE);
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	void TuningSourceHandler::setFotipProcessingNumerator(RupFotip* reply)
	{
		TEST_PTR_RETURN(reply);

		if (m_fotipVersion >= Fotip::V3)
		{
			quint64 numerator = m_processingNumeratorsMap.getNextFotipProcessingNumerator(m_lmEquipmentID);
			reply->fotipFrame.header.fotipProcessingNumerator = numerator;
		}
		else
		{
			reply->fotipFrame.header.fotipProcessingNumerator = 0;
		}
	}


}

