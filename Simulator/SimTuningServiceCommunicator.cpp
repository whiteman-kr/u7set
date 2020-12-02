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

	bool TuningServiceCommunicator::updateTuningRam(const QString& lmEquipmentId, const QString& portEquipmentId, const RamArea& ramArea, TimeStamp timeStamp)
	{
		qDebug() << "TuningServiceCommunicator::updateTuningRam LM: " << lmEquipmentId << ", Port: " << portEquipmentId << ", TimeStamp: " << timeStamp.toDateTime();

		// This function is called by LM after each workcycle
		// data: contains tuning memory area
		//
		int yuriy_beliy_to_do;	// Take ramArea and return as fast as possible
		return true;
	}

	void TuningServiceCommunicator::tuningModeChanged(const QString& lmEquipmentId, bool tuningMode)
	{
		qDebug() << "TuningServiceCommunicator::tuningModeChanged, LM: " << lmEquipmentId << ", value = " << tuningMode;
	}

	void TuningServiceCommunicator::writeTuningWord(const QString& lmEquipmentId, const QString& portEquipmentId, quint16 data, quint16 mask)
	{
		return writeTuningRecord(TuningRecord::createWord(lmEquipmentId, portEquipmentId, data, mask));
	}

	void TuningServiceCommunicator::writeTuningDword(const QString& lmEquipmentId, const QString& portEquipmentId, quint32 data)
	{
		return writeTuningRecord(TuningRecord::createDword(lmEquipmentId, portEquipmentId, data));
	}

	void TuningServiceCommunicator::writeTuningSignedInt32(const QString& lmEquipmentId, const QString& portEquipmentId, qint32 data)
	{
		return writeTuningRecord(TuningRecord::createSignedInt32(lmEquipmentId, portEquipmentId, data));
	}

	void TuningServiceCommunicator::writeTuningFloat(const QString& lmEquipmentId, const QString& portEquipmentId, float data)
	{
		return writeTuningRecord(TuningRecord::createFloat(lmEquipmentId, portEquipmentId, data));
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

		initTuningSourcesInterfaces(settings);
	}

	TuningRequestsProcessingThread::~TuningRequestsProcessingThread()
	{
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

			msleep(100);
		}

		closeSocket();

		m_log.writeMessage(QString("TuningRequestsProcessingThread is finished (EquipmentID %1)").
							arg(m_tuningServiceEquipmentID));
	}

	void TuningRequestsProcessingThread::initTuningSourcesInterfaces(const TuningServiceSettings& settings)
	{
		m_tuningSources.clear();

		for(const TuningServiceSettings::TuningSource& ts : settings.sources)
		{
			std::shared_ptr<LogicModule> lm = m_sim->logicModule(ts.equipmentID);

			if (lm == nullptr)
			{
				m_log.writeError(QString("Tuning source %1 isn't initialized").arg(ts.equipmentID));
				continue;
			}

			TuningSourceInterface tsi(m_tsCommunicator,
									  ts.equipmentID,
									  ts.tuningDataIP,
									  lm->logicModuleExtraInfo());

			m_tuningSources.insert({ts.tuningDataIP.address32(), tsi});
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
				msleep(200);
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

			auto ts = m_tuningSources.find(tuningSourceIP);

			if (ts == m_tuningSources.end())
			{
				continue;
			}

			TuningSourceInterface& tsInterface = ts->second;

			bool result = tsInterface.processRequest(request.rupFotipV2, &reply.rupFotipV2);

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

	TuningSourceInterface::TuningSourceInterface(TuningServiceCommunicator* tsCommunicator,
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

		m_tuningFlashSizeB = lmDescription.flashMemory().m_tuningFrameCount * lmDescription.flashMemory().m_tuningFrameSize;
		m_tuningFlashFramePayloadB = lmDescription.flashMemory().m_tuningFramePayload;
	}

	TuningSourceInterface::~TuningSourceInterface()
	{
	}

	bool TuningSourceInterface::processRequest(const RupFotipV2& request, RupFotipV2* reply)
	{
		Rup::Header requestRupHeader = request.rupHeader;

		requestRupHeader.reverseBytes();

		FotipV2::Header requestFotipHeader = request.fotipFrame.header;

		requestFotipHeader.reverseBytes();

		int to_do_add_requestRupHeader_checking;
		int to_do_add_requestFotipV2Header_checking;

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
		FotipV2::Header& replyFotipHeader = reply->fotipFrame.header;

		replyFotipHeader.protocolVersion = FotipV2::VERSION;
		replyFotipHeader.uniqueId = m_lmUniqueID;
		replyFotipHeader.subsystemKey.lmNumber = m_lmNumber;
		replyFotipHeader.subsystemKey.subsystemCode = m_subsystemKey;
		replyFotipHeader.operationCode = requestFotipHeader.operationCode;
		replyFotipHeader.fotipFrameSizeB = sizeof(FotipV2::Frame);
		replyFotipHeader.romSizeB = m_tuningFlashSizeB;
		replyFotipHeader.romFrameSizeB = m_tuningFlashFramePayloadB;

		FotipV2::HeaderFlags& flags = replyFotipHeader.flags;

		flags.all = 0;

		replyFotipHeader.reverseBytes();

		// check FOTIP error flags
		//
/*		if (flags.dataTypeError == 1)
		{
			m_stat.fotipFlagDataTypeErr++;
			result = false;
		}

		if (flags.operationCodeError == 1)
		{
			m_stat.fotipFlagOpCodeErr++;
			result = false;
		}

		if (flags.startAddressError == 1)
		{
			m_stat.fotipFlagStartAddrErr++;
			result = false;
		}

		if (flags.romSizeError == 1)
		{
			m_stat.fotipFlagRomSizeErr++;
			result = false;
		}

		if (flags.romFrameSizeError == 1)
		{
			m_stat.fotipFlagRomFrameSizeErr++;
			result = false;
		}

		if (flags.frameSizeError == 1)
		{
			m_stat.fotipFlagFrameSizeErr++;
			result = false;
		}

		if (flags.versionError == 1)
		{
			m_stat.fotipFlagProtocolVersionErr++;
			result = false;
		}

		if (flags.subsystemKeyError == 1)
		{
			m_stat.fotipFlagSubsystemKeyErr++;
			result = false;
		}

		if (flags.idError == 1)
		{
			m_stat.fotipFlagUniueIDErr++;
			result = false;
		}

		if (flags.offsetError == 1)
		{
			m_stat.fotipFlagOffsetErr++;
			result = false;
		}

		// check FOTIP success flags
		//
		if (flags.successfulCheck == 1)
		{
			m_stat.fotipFlagBoundsCheckSuccess++;
		}

		if (flags.successfulWrite == 1)
		{
			m_stat.fotipFlagWriteSuccess++;
		}

		if (flags.succesfulApply == 1)
		{
			m_stat.fotipFlagApplySuccess++;
		}

		if (flags.setSOR == 1)
		{
			m_stat.fotipFlagSetSOR++;					// for platform LMs
			m_stat.setSOR = true;

			m_stat.fotipFlagWritingDisabled++;			// for non-platform LMs
			m_stat.writingDisabled = true;
		}
		else
		{
			m_stat.fotipFlagSetSOR = 0;					// for platform LMs
			m_stat.setSOR = false;

			m_stat.fotipFlagWritingDisabled = 0;		// for non-platform LMs
			m_stat.writingDisabled = false;
		}

*/
		reply->calcCRC64();

		return true;
	}
}

