#include "SimTuningServiceCommunicator.h"
#include "Simulator.h"

namespace Sim
{

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningServiceCommunicator class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningServiceCommunicator::TuningServiceCommunicator(Simulator* simulator, const ::TuningServiceSettings& settings) :
		m_simulator(simulator),
		m_log(m_simulator->log(), "TuningCommunicator"),
		m_settings(settings)
	{
		Q_ASSERT(simulator);

		connect(m_simulator, &Simulator::projectUpdated, this, &TuningServiceCommunicator::projectUpdated);

		return;
	}

	TuningServiceCommunicator::~TuningServiceCommunicator()
	{
		stopProcessingThread();
	}

	bool TuningServiceCommunicator::startSimulation()
	{
		startProcessingThread();

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
													TimeStamp timeStamp)
	{
		if (softwareEnabled() == false)
		{
			return false;
		}

		if (m_processingThread != nullptr)
		{
			m_processingThread->updateTuningData(lmEquipmentId, portEquipmentId, ramArea, timeStamp);
		}

		return true;
	}

	void TuningServiceCommunicator::writeConfirmation(std::vector<qint64> confirmedRecords,
													  const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  TimeStamp timeStamp)
	{
		int to_Yuriy_Beliy_get_write_confirmation_here;
	}

	void TuningServiceCommunicator::tuningModeEntered(const QString& lmEquipmentId,
													  const QString& portEquipmentId,
													  const RamArea& ramArea,
													  TimeStamp timeStamp)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->tuningModeEntered(lmEquipmentId, portEquipmentId, ramArea, timeStamp);
		}
	}

	void TuningServiceCommunicator::tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->tuningModeLeft(lmEquipmentId, portEquipmentId);
		}
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

	void TuningServiceCommunicator::startProcessingThread()
	{
		Q_ASSERT(m_processingThread == nullptr);

		m_processingThread = new TuningRequestsProcessingThread(this, m_settings);
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

	TuningRequestsProcessingThread::TuningRequestsProcessingThread(TuningServiceCommunicator* tsCommunicator,
																   const TuningServiceSettings& settings) :
		m_tsCommunicator(tsCommunicator),
		m_sim(tsCommunicator->simulator()),
		m_log(m_tsCommunicator->simulator()->log()),
		m_tuningServiceEquipmentID(settings.equipmentID),
		m_tuningRequestsReceivingIP(settings.tuningSimIP),
		m_tuningRepliesSendingIP(settings.tuningDataIP)
	{
		m_tuningRequestsReceivingIP.setAddressPortStr(m_tuningRepliesSendingIP.addressStr(), PORT_LM_TUNING);

		initTuningSourcesHandlers(settings);
	}

	TuningRequestsProcessingThread::~TuningRequestsProcessingThread()
	{
	}

	void TuningRequestsProcessingThread::updateTuningData(const QString& lmEquipmentID,
														  const QString& portEquipmentID,
														  const RamArea& data,
														  TimeStamp timeStamp)
	{
		auto p = m_tuningSourcesByEquipmentID.find(std::pair<QString, QString>(lmEquipmentID, portEquipmentID));

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			Q_ASSERT(false);
			return;
		}

		p->second->updateTuningData(data, timeStamp);
	}

	void TuningRequestsProcessingThread::tuningModeEntered(const QString& lmEquipmentId,
														   const QString& portEquipmentId,
														   const RamArea& ramArea,
														   TimeStamp timeStamp)
	{
		auto p = m_tuningSourcesByEquipmentID.find(std::pair<QString, QString>(lmEquipmentId, portEquipmentId));

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			Q_ASSERT(false);
			return;
		}

		p->second->tuningModeEntered(ramArea, timeStamp);
	}

	void TuningRequestsProcessingThread::tuningModeLeft(const QString& lmEquipmentId, const QString& portEquipmentId)
	{
		auto p = m_tuningSourcesByEquipmentID.find(std::pair<QString, QString>(lmEquipmentId, portEquipmentId));

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			Q_ASSERT(false);
			return;
		}

		p->second->tuningModeLeft();
	}

	void TuningRequestsProcessingThread::run()
	{
		m_log.writeMessage(QString("Tuning simulation is started (EquipmentID %1, receiving IP %2, sending IP %3)").
						arg(m_tuningServiceEquipmentID).
						arg(m_tuningRequestsReceivingIP.addressPortStr()).
						arg(m_tuningRepliesSendingIP.addressPortStr()));

		m_thisThread = QThread::currentThread();

		while(isQuitRequested() == false)
		{
			bool result = tryCreateAndBindSocket();

			if (result == false)
			{
				continue;
			}

			receiveRequests();
		}

		closeSocket();

		m_log.writeMessage(QString("TuningRequestsProcessingThread is finished (EquipmentID %1)").
							arg(m_tuningServiceEquipmentID));
	}

	void TuningRequestsProcessingThread::initTuningSourcesHandlers(const TuningServiceSettings& settings)
	{
		m_tuningSourcesByIP.clear();
		m_tuningSourcesByEquipmentID.clear();

		for(const TuningServiceSettings::TuningSource& ts : settings.sources)
		{
			std::shared_ptr<LogicModule> lm = m_sim->logicModule(ts.lmEquipmentID);

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

	bool TuningRequestsProcessingThread::tryCreateAndBindSocket()
	{
		if (m_socket != nullptr)
		{
			closeSocket();
		}

		qint64 prevServerTime = -1;

		while(isQuitRequested() == false)
		{
			qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

			if (prevServerTime != -1 && serverTime - prevServerTime < 1000)
			{
				msleep(100);
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

				msleep(200);

				continue;
			}

			// bind Ok

			m_log.writeMessage(QString("Tuning simulation listening socket is created and bound to %1").
							   arg(m_tuningRequestsReceivingIP.addressPortStr()));

			QVariant newRecvBufSize(static_cast<int>(2 * 1024 * 1024));

			m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

			QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

			if (newRecvBufSize.toInt() != currentBufSize.toInt())
			{
				m_log.writeWarning(QString("Tuning simulation receive buffer size is not changed to required size."));
			}

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

		QHostAddress from;

		const int BUFFER_SIZE = sizeof(SimRupFotipV2) + 1;

		char receiveBuffer[BUFFER_SIZE];

		SimRupFotipV2& request = *reinterpret_cast<SimRupFotipV2*>(receiveBuffer);

		qint64 lastRequestTime = QDateTime::currentMSecsSinceEpoch();

		SimRupFotipV2 reply;

		while(isQuitRequested() == false)
		{
			qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

			qint64 size = m_socket->readDatagram(receiveBuffer, BUFFER_SIZE, &from);

			if (size == -1)
			{
				if (serverTime - lastRequestTime > 3000)
				{
					// recreate socket if has no requests in 3 seconds
					//
					closeSocket();
					return;
				}

				msleep(5);
				continue;
			}

			lastRequestTime = serverTime;

			if (size != sizeof(SimRupFotipV2))
			{
				Q_ASSERT(false);
				continue;
			}

			if (request.rupFotipV2.checkCRC64() == false)
			{
				Q_ASSERT(false);
				continue;
			}

			quint16 simVersion = reverseUint16(request.simVersion);

			if (simVersion != 1)
			{
				Q_ASSERT(false);
				continue;
			}

			quint32 tuningSourceIP = reverseUint32(request.tuningSourceIP);

			auto ts = m_tuningSourcesByIP.find(tuningSourceIP);

			if (ts == m_tuningSourcesByIP.end())
			{
				continue;
			}

			bool result = ts->second->processRequest(request.rupFotipV2, &reply.rupFotipV2);

			if (result == false)
			{
				continue;				// reply will not be send
			}

			reply.simVersion = reverseUint16(1);
			reply.tuningSourceIP = reverseUint32(tuningSourceIP);

			m_socket->writeDatagram(reinterpret_cast<const char*>(&reply),
									sizeof(reply),
									m_tuningRepliesSendingIP.address(),
									m_tuningRepliesSendingIP.port());
		}
	}

	// ---------------------------------------------------------------------------------------------------------
	//
	// TuningSourceInterface class implementation
	//
	// ---------------------------------------------------------------------------------------------------------

	TuningSourceHandler::TuningSourceHandler(TuningServiceCommunicator* tsCommunicator,
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
		std::shared_ptr<LogicModule> lm = m_tsCommunicator->simulator()->logicModule(m_lmEquipmentID);

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

	void TuningSourceHandler::updateTuningData(const RamArea& data, TimeStamp timeStamp)
	{
		Q_UNUSED(timeStamp);

		m_tuningDataMutex.lock();

		*m_tuningData.get() = data;

		m_tuningDataMutex.unlock();
	}

	void TuningSourceHandler::tuningModeEntered(const RamArea& ramArea, TimeStamp timeStamp)
	{
		updateTuningData(ramArea, timeStamp);

		m_tuningEnabled.store(true);
	}

	void TuningSourceHandler::tuningModeLeft()
	{
		m_tuningEnabled.store(false);
	}

	bool TuningSourceHandler::processRequest(const RupFotipV2& request, RupFotipV2* reply)
	{
		if (m_tuningEnabled == false)
		{
			return false;			// no send replies while tuning disabled
		}

		Rup::Header requestRupHeader = request.rupHeader;

		requestRupHeader.reverseBytes();

		bool sendReply = true;

		sendReply = checkRequestRupHeader(requestRupHeader);

		if (sendReply == false)
		{
			// reply will not be send if Rup::Header errors were detected
			//
			return false;
		}

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

		replyRupHeader.reverseBytes();

		// reply FotipV2::Header initialization
		//
		FotipV2::Header requestFotipHeader = request.fotipFrame.header;

		requestFotipHeader.reverseBytes();

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

		memset(replyFotipHeader.reserv, 0, sizeof(replyFotipHeader.reserv));

		reply->fotipFrame.analogCmpErrors.all = 0;

		memset(reply->fotipFrame.data, 0, sizeof(reply->fotipFrame.data));
		memset(reply->fotipFrame.reserv, 0, sizeof(reply->fotipFrame.reserv));

		reply->fotipFrame.header.startAddressW = requestFotipHeader.startAddressW;
		reply->fotipFrame.header.offsetInFrameW = requestFotipHeader.offsetInFrameW;

		FotipV2::HeaderFlags replyFlags;

		replyFlags.all = 0;

		bool processRequest = checkRequestFotipHeader(requestFotipHeader, &replyFlags);

		if (processRequest == false)
		{
			// Any error in requestFotipHeader disable request processing
			//
		}
		else
		{
			//
//				quint16 successfulCheck : 1;
//			quint16 successfulWrite : 1;
//			quint16 succesfulApply : 1;

			switch(static_cast<FotipV2::OpCode>(requestFotipHeader.operationCode))
			{
			case FotipV2::OpCode::Read:
				processReadRequest(request.fotipFrame, &reply->fotipFrame, &replyFlags);
				break;

			case FotipV2::OpCode::Write:
				processWriteRequest(request.fotipFrame, &reply->fotipFrame, &replyFlags);
				break;

			case FotipV2::OpCode::Apply:
				processApplyRequest(request.fotipFrame, &reply->fotipFrame, &replyFlags);
				break;

			default:
				Q_ASSERT(false);
			}

			//			quint16 setSOR : 1;				// for non-platform modules 1 in this flag means "WritingDisabled"
			int to_do_SOR_flag_requesting;
		}

		replyFotipHeader.flags = replyFlags;

		replyFotipHeader.reverseBytes();

		reply->calcCRC64();

		return true;
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

	bool TuningSourceHandler::checkRequestFotipHeader(const FotipV2::Header& fotipHeader, FotipV2::HeaderFlags* replyFlags)
	{
		if (fotipHeader.protocolVersion != FotipV2::VERSION)
		{
			replyFlags->versionError = 1;
		}

		if (fotipHeader.uniqueId != m_lmUniqueID)
		{
			replyFlags->idError = 1;
		}

		if (fotipHeader.subsystemKey.lmNumber != m_lmNumber ||
			fotipHeader.subsystemKey.subsystemCode != m_subsystemKey)
		{
			replyFlags->subsystemKeyError = 1;
		}

		if (fotipHeader.fotipFrameSizeB != sizeof(FotipV2::Frame))
		{
			replyFlags->frameSizeError = 1;
		}

		if (fotipHeader.romSizeB !=  m_tuningFlashSizeB)
		{
			replyFlags->romSizeError = 1;
		}

		if (fotipHeader.romFrameSizeB != m_tuningFlashFramePayloadB)
		{
			replyFlags->romFrameSizeError = 1;
		}

		switch(static_cast<FotipV2::OpCode>(fotipHeader.operationCode))
		{
		case FotipV2::OpCode::Read:
		case FotipV2::OpCode::Write:
		case FotipV2::OpCode::Apply:
			break;

		default:
			replyFlags->operationCodeError = 1;
		}

		switch(static_cast<FotipV2::DataType>(fotipHeader.dataType))
		{
		case FotipV2::DataType::AnalogSignedInt:
		case FotipV2::DataType::AnalogFloat:
		case FotipV2::DataType::Discrete:
			break;

		default:
			replyFlags->dataTypeError = 1;
		};

		if (fotipHeader.startAddressW < m_tuningDataStartAddrW ||
			fotipHeader.startAddressW > (m_tuningDataStartAddrW + m_tuningDataSizeW) ||
			((fotipHeader.startAddressW - m_tuningDataStartAddrW) % m_tuningDataFramePayloadW) != 0)
		{
			replyFlags->startAddressError = 1;
		}

		if (fotipHeader.offsetInFrameW >= m_tuningDataFramePayloadW ||	// "equal" in condition is Ok!
			(fotipHeader.offsetInFrameW % 2) != 0 )						// possible offsetInFrameW values - even in range 0..507 only
		{
			replyFlags->offsetError = 1;
		}

		return replyFlags->all == 0;
	}

	void TuningSourceHandler::processReadRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags)
	{
		quint32 requestedTuningDataStartAddrW = reverseUint32(request.header.startAddressW);

		readFrameData(requestedTuningDataStartAddrW, reply);
	}

	void TuningSourceHandler::processWriteRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags)
	{
		quint32 frameStartAddrW = reverseUint32(request.header.startAddressW);
		quint32 offsetInFrameW = reverseUint32(request.header.offsetInFrameW);

		quint32 writeAddrW = frameStartAddrW + offsetInFrameW;

		replyFlags->successfulWrite = 1;

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
					m_tsCommunicator->writeTuningSignedInt32(m_lmEquipmentID, m_portEquipmentID, writeAddrW, value);
					replyFlags->successfulCheck = 1;
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

					replyFlags->successfulCheck = 0;
					replyFlags->successfulWrite = 0;
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
					m_tsCommunicator->writeTuningFloat(m_lmEquipmentID, m_portEquipmentID, writeAddrW, value);
					replyFlags->successfulCheck = 1;
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

					replyFlags->successfulCheck = 0;
					replyFlags->successfulWrite = 0;
				}

				m_tsCommunicator->writeTuningFloat(m_lmEquipmentID, m_portEquipmentID, writeAddrW, value);
			}

			break;

		case FotipV2::DataType::Discrete:
			{
				quint32 value = reverseUint32(request.write.discreteValue);
				quint32 mask = reverseUint32(request.write.bitMask);

				m_tsCommunicator->writeTuningDword(m_lmEquipmentID, m_portEquipmentID, writeAddrW, value, mask);

				replyFlags->successfulCheck = 1;
			}

			break;

		default:
			replyFlags->successfulWrite = 0;
		}

		readFrameData(frameStartAddrW, reply);
	}

	void TuningSourceHandler::processApplyRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags)
	{
		replyFlags->succesfulApply = 1;

		int to_do_real_APPLY_processing;
	}

	void TuningSourceHandler::readFrameData(quint32 startFrameAddrW, FotipV2::Frame* reply)
	{
		m_tuningDataMutex.lock();

		bool res = m_tuningData->readToBuffer<std::vector<quint8>>(startFrameAddrW,
																   m_tuningDataFramePayloadW,
																   &m_tuningDataReadBuffer,
																   false);
		DEBUG_STOP;

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

