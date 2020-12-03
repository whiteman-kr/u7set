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
		if (m_processingThread != nullptr)
		{
			m_processingThread->updateTuningData(lmEquipmentId, portEquipmentId, ramArea, timeStamp);
		}

		return true;
	}

	void TuningServiceCommunicator::tuningModeChanged(const QString& lmEquipmentId, bool tuningMode)
	{
		if (m_processingThread != nullptr)
		{
			m_processingThread->tuningModeChanged(lmEquipmentId, tuningMode);
		}
	}

	void TuningServiceCommunicator::writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, quint32 data, quint32 mask)
	{
		return writeTuningRecord(TuningRecord::createDword(lmEquipmentId, portEquipmentId, offsetW, data, mask));
	}

	void TuningServiceCommunicator::writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, qint32 data)
	{
		return writeTuningRecord(TuningRecord::createSignedInt32(lmEquipmentId, portEquipmentId, offsetW, data));
	}

	void TuningServiceCommunicator::writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 offsetW, float data)
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

	void TuningServiceCommunicator::writeTuningRecord(TuningRecord&& r)
	{
		QMutexLocker l(&m_qmutex);

		m_writeTuningQueue[r.lmEquipmentId].push(std::move(r));

		return;
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

	bool TuningServiceCommunicator::enabled() const
	{
		return m_enabled;
	}

	void TuningServiceCommunicator::TuningServiceCommunicator::setEnabled(bool value)
	{
		m_enabled = value;
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
		Q_UNUSED(portEquipmentID);

		auto p = m_tuningSourcesByEquipmentID.find(lmEquipmentID);

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			Q_ASSERT(false);
			return;
		}

		p->second->updateTuningData(data, timeStamp);
	}

	void TuningRequestsProcessingThread::tuningModeChanged(const QString& lmEquipmentId, bool tuningEnabled)
	{
		auto p = m_tuningSourcesByEquipmentID.find(lmEquipmentId);

		if (p == m_tuningSourcesByEquipmentID.end())
		{
			Q_ASSERT(false);
			return;
		}

		p->second->tuningModeChanged(tuningEnabled);
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
			std::shared_ptr<LogicModule> lm = m_sim->logicModule(ts.equipmentID);

			if (lm == nullptr)
			{
				m_log.writeError(QString("Tuning source %1 isn't initialized").arg(ts.equipmentID));
				continue;
			}

			auto tsh = std::make_shared<TuningSourceHandler>(m_tsCommunicator,
															ts.equipmentID,
															ts.tuningDataIP,
															lm->logicModuleExtraInfo());

			m_tuningSourcesByIP.insert({ts.tuningDataIP.address32(), tsh});
			m_tuningSourcesByEquipmentID.insert({ts.equipmentID, tsh});
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
				m_log.writeAlert(QString("Tuning simulation listening socket binding error to %1").
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
												 const QString& equipmentID,
												 const HostAddressPort& ip,
												 const ::LogicModuleInfo& logicModuleInfo) :
		m_tsCommunicator(tsCommunicator),
		m_equipmentID(equipmentID),
		m_tuningSourceIP(ip),
		m_moduleType(logicModuleInfo.moduleType()),
		m_lmNumber(logicModuleInfo.lmNumber),
		m_subsystemKey(logicModuleInfo.subsystemKey),
		m_lmUniqueID(logicModuleInfo.lmUniqueID)
	{
		std::shared_ptr<LogicModule> lm = m_tsCommunicator->simulator()->logicModule(m_equipmentID);

		TEST_PTR_RETURN(lm);

		const LmDescription& lmDescription = lm->lmDescription();

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
													QString("TuningData::") + m_equipmentID);

		m_tuningDataReadBuffer.resize(m_tuningDataFramePayloadB);
	}

	TuningSourceHandler::~TuningSourceHandler()
	{
	}

	void TuningSourceHandler::updateTuningData(const RamArea& data, TimeStamp timeStamp)
	{
		Q_UNUSED(timeStamp);

		if (m_tuningEnabled == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_tuningDataMutex.lock();

		*m_tuningData.get() = data;

		m_tuningDataMutex.unlock();
	}

	void TuningSourceHandler::tuningModeChanged(bool tuningEnabled)
	{
		m_tuningEnabled.store(tuningEnabled);
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
		replyFotipHeader.fotipFrameSizeB = sizeof(FotipV2::Frame);
		replyFotipHeader.romSizeB = m_tuningDataSizeB;
		replyFotipHeader.romFrameSizeB = m_tuningDataFramePayloadB;

		memset(replyFotipHeader.reserv, 0, sizeof(replyFotipHeader.reserv));

		reply->fotipFrame.analogCmpErrors.all = 0;

		memset(reply->fotipFrame.data, 0, sizeof(reply->fotipFrame.data));
		memset(reply->fotipFrame.reserv, 0, sizeof(reply->fotipFrame.reserv));

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

		if (fotipHeader.romSizeB !=  m_tuningDataSizeB)
		{
			replyFlags->romSizeError = 1;
		}

		if (fotipHeader.romFrameSizeB != m_tuningDataFramePayloadB)
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
			((fotipHeader.startAddressW - m_tuningDataStartAddrW) % m_tuningDataFrameSizeW) != 0)
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
		quint32 requestedTuningDataStartAddrW = request.header.startAddressW;

		m_tuningDataMutex.lock();

		bool res = m_tuningData->readToBuffer<std::vector<quint8>>(requestedTuningDataStartAddrW,
																   m_tuningDataFramePayloadW,
																   &m_tuningDataReadBuffer,
																   false);
		Q_ASSERT(res == true);

		m_tuningDataMutex.unlock();

		if (m_tuningDataReadBuffer.size() == FotipV2::TX_RX_DATA_SIZE)
		{
			memcpy(reply->data, m_tuningDataReadBuffer.data(), FotipV2::TX_RX_DATA_SIZE);
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	void TuningSourceHandler::processWriteRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags)
	{
		replyFlags->successfulWrite = 1;

		int to_do_real_WRITE_processing;
	}

	void TuningSourceHandler::processApplyRequest(const FotipV2::Frame& request, FotipV2::Frame* reply, FotipV2::HeaderFlags* replyFlags)
	{
		replyFlags->succesfulApply = 1;

		int to_do_real_APPLY_processing;
	}
}

