#include "TcpTuningServer.h"
#include "TuningService.h"
#include "../OnlineLib/TcpFileTransfer.h"

namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServer class implementation
	//
	// -------------------------------------------------------------------------------

	const QString TcpTuningServer::SCM_CLIENT_ID("SCM");
	quint64 TcpTuningServer::m_staticTcpConnectionID = 0;

	TcpTuningServer::TcpTuningServer(TuningServiceWorker& service,
		const TuningSources& tuningSources,
		std::shared_ptr<std::vector<char>> tuningSourcesFileData,
		std::shared_ptr<CircularLogger> logger) :
		Tcp::Server(service.softwareInfo(), "TcpTuningServer"),
		m_service(service),
		m_tuningSources(tuningSources),
		m_tuningSourcesFileData(tuningSourcesFileData),
		m_logger(logger)
	{
		m_tcpConnectionID = ++m_staticTcpConnectionID;

		Q_ASSERT(m_tuningSourcesFileData);

		if (m_tuningSourcesFileData != nullptr && m_tuningSourcesFileData->empty() == false)
		{
			m_tuningSourcesFileCrc64 = Crc::crc64(m_tuningSourcesFileData->data(),
											 TO_QINT64(m_tuningSourcesFileData->size()));
		}

		prepareSignalGetter();
	}

	void TcpTuningServer::onServerThreadStarted()
	{
	}

	void TcpTuningServer::onServerThreadFinished()
	{
	}

	void TcpTuningServer::onConnection()
	{
		Tcp::Server::onConnection();

		m_clientEquipmentID = connectedSoftwareInfo().equipmentID();
		m_service.registerSignalsStateChangesQueue(m_clientEquipmentID, m_tcpConnectionID);
	}

	void TcpTuningServer::onDisconnection()
	{
		Tcp::Server::onDisconnection();

		m_service.unregisterSignalsStateChangesQueue(m_clientEquipmentID, m_tcpConnectionID);

		m_service.clientIsDisconnected(connectedSoftwareInfo(), peerAddr().addressStr());
	}

	Tcp::Server* TcpTuningServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
	{
		TcpTuningServer* newServer =  new TcpTuningServer(m_service, m_tuningSources,
														 m_tuningSourcesFileData, m_logger);
		newServer->setListenAddress(listenAddr);
		return newServer;
	}

	void TcpTuningServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		switch(requestID)
		{
		case TDS_GET_TUNING_SOURCES_INFO:
			onGetTuningSourcesInfoRequest(requestData, requestDataSize);
			break;

		case TDS_GET_TUNING_SOURCES_STATES:
			onGetTuningSourcesStateRequest(requestData, requestDataSize);
			break;

		case TDS_GET_TUNING_SOURCE_FILLING:
			onGetTuningSourceFilling(requestData, requestDataSize);
			break;

		case TDS_GET_TUNING_SIGNAL_PARAM:
			onGetTuningSignalParam(requestData, requestDataSize);
			break;

		case TDS_TUNING_SIGNALS_READ:
			onTuningSignalsReadRequest(requestData, requestDataSize);
			break;

		case TDS_GET_SIGNALS_STATE_CHANGES:
			onGetTuningSignalsStateChangesRequest(requestData, requestDataSize);
			break;

		case TDS_TUNING_SIGNALS_WRITE:
			onTuningSignalsWriteRequest(requestData, requestDataSize);
			break;

		case TDS_TUNING_SIGNALS_APPLY:
			onTuningSignalsApplyRequest(requestData, requestDataSize);
			break;

		case TDS_CHANGE_CONTROLLED_TUNING_SOURCE:
			onChangeControlledTuningSourceRequest(requestData, requestDataSize);
			break;

		case TDS_GET_TUNING_SERVICE_SETTINGS:
			onGetTuningServiceSettings(requestData, requestDataSize);
			break;

		case RQID_GET_CLIENT_LIST:
			sendClientList();
			break;

		case TDS_GET_TUNING_SOURCES_FILE:
			onGetTuningSourcesFile(requestData, requestDataSize);
			break;

		default:
			logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
		}
	}

	void TcpTuningServer::onConnectedSoftwareInfoChanged()
	{
		m_service.clientIsConnected(connectedSoftwareInfo(), peerAddr().addressStr());
	}

	void TcpTuningServer::onGetTuningSourcesInfoRequest(const char* requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesInfoReply.Clear();

		bool result = m_getTuningSourcesInfo.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_getTuningSourcesInfoReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_getTuningSourcesInfoReply);
			return;
		}

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_GET_TUNING_SOURCES_INFO request from %1, %2")).
					  arg(m_clientEquipmentID).arg(peerAddr().addressStr()));

		E::NetworkError errCode = E::NetworkError::Success;

		const TuningClientContext* clntContext =
				m_service.getClientContext(m_clientEquipmentID);

		if (clntContext == nullptr &&
			m_clientEquipmentID != SCM_CLIENT_ID)
		{
			// unknown clientID
			//
			errCode = E::NetworkError::UnknownTuningClientID;

			m_getTuningSourcesInfoReply.set_error(TO_INT(errCode));
			sendReply(m_getTuningSourcesInfoReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_INFO to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_clientSourcesList.has_value() == false)
		{
			initClientSourcesList(m_clientEquipmentID);
		}

		for(const QString& sourceID : m_clientSourcesList.value())
		{
			const TuningSource* src = m_tuningSources.getSourceByID(sourceID);

			TEST_PTR_CONTINUE(src);

			Network::DataSourceInfo* newDsi = m_getTuningSourcesInfoReply.add_tuningsourceinfo();

			TEST_PTR_CONTINUE(newDsi);

			src->saveToProto(newDsi);
		}

		errCode = E::NetworkError::Success;

		m_getTuningSourcesInfoReply.set_singlelmcontrolmode(m_service.singleLmControl());
		m_getTuningSourcesInfoReply.set_activeclientid(m_service.activeClientID().toStdString());
		m_getTuningSourcesInfoReply.set_activeclientip(m_service.activeClientIP().toStdString());

		m_getTuningSourcesInfoReply.set_error(TO_INT(errCode));

		sendReply(m_getTuningSourcesInfoReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_INFO to %2")).
					  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
	}

	void TcpTuningServer::onGetTuningSourcesStateRequest(const char* requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesStatesReply.Clear();

		bool result = m_getTuningSourcesStates.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_getTuningSourcesStatesReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_getTuningSourcesStatesReply);
			return;
		}

		const TuningClientContext* clntContext =
				m_service.getClientContext(m_clientEquipmentID);

		if (clntContext == nullptr &&
			m_clientEquipmentID != SCM_CLIENT_ID)
		{
			// unknown clientID
			//
			E::NetworkError errCode = E::NetworkError::UnknownTuningClientID;

			m_getTuningSourcesStatesReply.set_error(TO_INT(errCode));
			sendReply(m_getTuningSourcesStatesReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_STATES to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_clientSourcesList.has_value() == false)
		{
			initClientSourcesList(m_clientEquipmentID);
		}

		for(const QString& sourceID : m_clientSourcesList.value())
		{
			const TuningSourceThreadShared sourceThread = m_service.getTuningSourceThread(sourceID);

			if (sourceThread == nullptr)
			{
				DEBUG_LOG_MSG(m_logger, QString("No sourceThread for %1").arg(sourceID));

				const TuningSource* src = m_tuningSources.getSourceByID(sourceID);

				TEST_PTR_CONTINUE(src);

				const QStringList& tuningLans = src->getEnabledLansProvidedTuning();

				for(const QString& lanID : tuningLans)
				{
					if (m_service.isControlled(src->moduleEquipmentID(), lanID) == false)
					{
						DEBUG_LOG_MSG(m_logger, QString("Module %1 not controlled (Lan %2)").arg(src->moduleEquipmentID()).arg(lanID));
						continue;
					}

					Network::TuningSourceState* newTss = m_getTuningSourcesStatesReply.add_tuningsourcesstate();

					newTss->set_sourceid(src->ID());
					newTss->set_moduleequipmentid(src->moduleEquipmentID().toStdString());
					newTss->set_lanequipmentid(lanID.toStdString());
					newTss->set_isreply(false);
				}

				DEBUG_LOG_MSG(m_logger,
							  QString("m_getTuningSourcesStatesReply states count = %1")
								  .arg(m_getTuningSourcesStatesReply.tuningsourcesstate_size()));
			}
			else
			{
				DEBUG_LOG_MSG(m_logger, QString("getSourceState for %1").arg(sourceID));

				bool res = sourceThread->getSourceState(&m_getTuningSourcesStatesReply);

				if (res == false)
				{
					DEBUG_LOG_MSG(m_logger, QString("No worker for %1").arg(sourceID));
				}

				DEBUG_LOG_MSG(m_logger,
							  QString("m_getTuningSourcesStatesReply states count = %1")
								  .arg(m_getTuningSourcesStatesReply.tuningsourcesstate_size()));
			}
		}

		m_getTuningSourcesStatesReply.set_singlelmcontrolmode(m_service.singleLmControl());
		m_getTuningSourcesStatesReply.set_activeclientid(m_service.activeClientID().toStdString());
		m_getTuningSourcesStatesReply.set_activeclientip(m_service.activeClientIP().toStdString());

		m_getTuningSourcesStatesReply.set_error(TO_INT(E::NetworkError::Success));

		DEBUG_LOG_MSG(m_logger,
					  QString(tr("Send reply on TDS_GET_TUNING_SOURCES_STATES, states count %1"))
						  .arg(m_getTuningSourcesStatesReply.tuningsourcesstate_size()));

		sendReply(m_getTuningSourcesStatesReply);
	}

	void TcpTuningServer::onTuningSignalsReadRequest(const char* requestData, quint32 requestDataSize)
	{
		m_tuningSignalsReadReply.Clear();

		bool result = m_tuningSignalsReadRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_tuningSignalsReadReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_tuningSignalsReadReply);
			return;
		}

		m_tuningSignalsReadReply.set_readrequestid(m_tuningSignalsReadRequest.readrequestid());
		m_tuningSignalsReadReply.set_pendingsignalsstatechanges(0);

		E::NetworkError errCode = E::NetworkError::Success;

		if (m_clientEquipmentID == SCM_CLIENT_ID)
		{
			int signalQuantity = m_tuningSignalsReadRequest.signalhash_size();

			m_tuningSignalsReadReply.clear_tuningsignalstate();

			for(int i = 0; i < signalQuantity; i++)
			{
				Network::TuningSignalState* tss = m_tuningSignalsReadReply.add_tuningsignalstate();

				if (tss == nullptr)
				{
					continue;
				}

				Hash signalHash = m_tuningSignalsReadRequest.signalhash(i);
				quint32 ip = m_signalHash2SourceIP.value(signalHash);

				const TuningSourceThreadShared thread = m_service.getTuningSourceThread(ip);

				TEST_PTR_CONTINUE(thread);

				tss->set_signalhash(signalHash);

				thread->readSignalState(tss);
			}

			m_tuningSignalsReadReply.set_error(TO_INT(E::NetworkError::Success));

			sendReply(m_tuningSignalsReadReply);
		}
		else
		{
			const TuningClientContext* clientContext =
					m_service.getClientContext(m_clientEquipmentID);

			if (clientContext == nullptr)
			{
				errCode = E::NetworkError::UnknownTuningClientID;

				m_tuningSignalsReadReply.set_error(TO_INT(errCode));

				sendReply(m_tuningSignalsReadReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
							  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
				return;
			}

			int signalCount = m_tuningSignalsReadRequest.signalhash_size();

			QElapsedTimer timer;
			timer.start();

			// m_tuningSignalsReadReply.set_error(???) is set inside clientContext->readSignalStates()
			//
			clientContext->readSignalStates(m_tuningSignalsReadRequest, &m_tuningSignalsReadReply);

			TuningSignalsChangesQueue* queue =
					m_service.getSignalChangesQueue(m_clientEquipmentID, m_tcpConnectionID);

			if (queue != nullptr)
			{
				m_tuningSignalsReadReply.set_pendingsignalsstatechanges(queue->size());
			}
			else
			{
				Q_ASSERT(false);
			}

			qint64 tm = timer.elapsed();

			qDebug() << C_STR(QString("READ %1 signals, time = %2").arg(signalCount).arg(tm));

			sendReply(m_tuningSignalsReadReply);
		}

		errCode = static_cast<E::NetworkError>(m_tuningSignalsReadReply.error());

		if (errCode != E::NetworkError::Success)
		{
			// log errors only
			//
			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
		}
	}

	void TcpTuningServer::onGetTuningSignalsStateChangesRequest(const char* requestData, quint32 requestDataSize)
	{
		Q_UNUSED(requestData);
		Q_UNUSED(requestDataSize);

		m_getStateChangesReply.Clear();

		E::NetworkError errCode = E::NetworkError::Success;

		TuningSignalsChangesQueue* queue =
				m_service.getSignalChangesQueue(m_clientEquipmentID, m_tcpConnectionID);

		if (queue == nullptr)
		{
			errCode = E::NetworkError::InternalError;

			m_getStateChangesReply.set_error(TO_INT(errCode));

			sendReply(m_getStateChangesReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_GET_SIGNALS_STATE_CHANGES to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		int count = 0;

		while(queue->isEmpty() == false && count < 10000)
		{
			Network::TuningSignalState* protoState = m_getStateChangesReply.add_tuningsignalstate();

			TuningSignal::State* state = queue->beginPop();

			state->saveToProto(protoState);

			queue->completePop();

			count++;
		}

		m_getStateChangesReply.set_pendingsignalsstatechanges(queue->size());
		m_getStateChangesReply.set_error(TO_INT(E::NetworkError::Success));

		sendReply(m_getStateChangesReply);

		errCode = static_cast<E::NetworkError>(m_getStateChangesReply.error());

		if (errCode != E::NetworkError::Success)
		{
			// log errors only
			//
			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
		}
	}

	void TcpTuningServer::onTuningSignalsWriteRequest(const char* requestData, quint32 requestDataSize)
	{
		m_tuningSignalsWriteReply.Clear();

		bool result = m_tuningSignalsWriteRequest.ParseFromArray(requestData, requestDataSize);

		m_tuningSignalsWriteReply.set_writerequestid(m_tuningSignalsWriteRequest.writerequestid());

		if (result == false)
		{
			m_tuningSignalsWriteReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_tuningSignalsWriteReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_TUNING_SIGNALS_WRITE request from client %1, %2 (Signals %3, AutoApply is %4)")).
					  arg(clientEquipmentID).
					  arg(peerAddr().addressStr()).
					  arg(m_tuningSignalsWriteRequest.commands_size()).
					  arg(m_tuningSignalsWriteRequest.autoapply() == true ? "TRUE" : "FALSE"));

		const TuningClientContext* clientContext = m_service.getClientContext(clientEquipmentID);

		E::NetworkError errCode = E::NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = E::NetworkError::UnknownTuningClientID;

			m_tuningSignalsWriteReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsWriteReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		E::SoftwareType swType = connectedSoftwareInfo().softwareType();

		if (swType == E::SoftwareType::TestSuite && m_tuningSignalsWriteRequest.autoapply() == true)
		{
			DEBUG_LOG_WRN(m_logger, QString(tr("Autoapply ignored (set to FALSE)")));
			m_tuningSignalsWriteRequest.set_autoapply(false);
		}

		if (m_service.singleLmControl() == true)
		{
			QString activeClientID = m_service.activeClientID();
			QString activeClientIP = m_service.activeClientIP();

			if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
			{
				errCode = E::NetworkError::ClientIsNotActive;

				m_tuningSignalsWriteReply.set_error(TO_INT(errCode));

				sendReply(m_tuningSignalsWriteReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
							  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
				return;
			}
		}

		QString matsUser = QString::fromStdString(m_tuningSignalsWriteRequest.matsuser());

		//QElapsedTimer timer;
		//timer.start();

		// m_tuningSignalsWriteReply.set_error(???) is set inside clientContext->ІwriteSignalStates()
		//
		clientContext->writeSignalStates(clientEquipmentID, matsUser, m_tuningSignalsWriteRequest, &m_tuningSignalsWriteReply);

		sendReply(m_tuningSignalsWriteReply);

		errCode = static_cast<E::NetworkError>(m_tuningSignalsWriteReply.error());

//		qDebug() << C_STR(QString("WRITE %1 signals, time = %2").arg(m_tuningSignalsWriteRequest.commands_size()).arg(timer.elapsed()));

		QString msg = QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
				arg(E::valueToString(errCode)).arg(peerAddr().addressStr());

		if (errCode == E::NetworkError::Success)
		{
			DEBUG_LOG_MSG(m_logger, msg);
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, msg);
		}
	}

	void TcpTuningServer::onTuningSignalsApplyRequest(const char* requestData, quint32 requestDataSize)
	{
		m_tuningSignalsApplyReply.Clear();

		bool result = m_tuningSignalsApplyRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_tuningSignalsApplyReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_tuningSignalsApplyReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_TUNING_SIGNALS_APPLY request from client %1, %2")).
					  arg(clientEquipmentID).
					  arg(peerAddr().addressStr()));

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientEquipmentID);

		E::NetworkError errCode = E::NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = E::NetworkError::UnknownTuningClientID;

			m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsApplyReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		E::SoftwareType swType = connectedSoftwareInfo().softwareType();

		if (swType == E::SoftwareType::TestSuite)
		{
			errCode = E::NetworkError::TuningCommandDenied;

			m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsApplyReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_service.singleLmControl() == true)
		{
			QString activeClientID = m_service.activeClientID();
			QString activeClientIP = m_service.activeClientIP();

			if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
			{
				errCode = E::NetworkError::ClientIsNotActive;

				m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

				sendReply(m_tuningSignalsApplyReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
							  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
				return;
			}
		}

		QString user = connectedSoftwareInfo().osUsername();

		clientContext->applySignalStates(clientEquipmentID, user);

		errCode = E::NetworkError::Success;

		m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

		sendReply(m_tuningSignalsApplyReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
					  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
	}

	void TcpTuningServer::onChangeControlledTuningSourceRequest(const char* requestData, quint32 requestDataSize)
	{
		m_changeControlledTuningSourceReply.Clear();

		bool result = m_changeControlledTuningSourceRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_changeControlledTuningSourceReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_changeControlledTuningSourceReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		const TuningClientContext* clientContext = m_service.getClientContext(clientEquipmentID);

		E::NetworkError errCode = E::NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = E::NetworkError::UnknownTuningClientID;

			m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

			sendReply(m_changeControlledTuningSourceReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_service.singleLmControl() == false)
		{
			errCode = E::NetworkError::SingleLmControlDisabled;

			m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

			sendReply(m_changeControlledTuningSourceReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
						  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		QString activeClientID = m_service.activeClientID();
		QString activeClientIP = m_service.activeClientIP();

		if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
		{
			if (m_changeControlledTuningSourceRequest.takecontrol() == false)
			{
				errCode = E::NetworkError::ClientIsNotActive;

				m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

				sendReply(m_changeControlledTuningSourceReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
							  arg(E::valueToString(errCode)).arg(peerAddr().addressStr()));
				return;
			}
			else
			{
				m_service.setActiveClient(connectedSoftwareInfo(), peerAddr().addressStr());
			}
		}

		QString tuningSourceEquipmentID = QString::fromStdString(m_changeControlledTuningSourceRequest.tuningsourceequipmentid());
		bool activateControl = m_changeControlledTuningSourceRequest.activatecontrol();

		QString controlledTuningSource;
		bool controlIsActive;

		E::NetworkError errorCode = m_service.changeControlledTuningSource(tuningSourceEquipmentID,
																		 activateControl,
																		 &controlledTuningSource,
																		 &controlIsActive);

		m_changeControlledTuningSourceReply.set_error(TO_INT(errorCode));
		m_changeControlledTuningSourceReply.set_controlledtuningsourceequipmentid(controlledTuningSource.toStdString());
		m_changeControlledTuningSourceReply.set_controlisactive(controlIsActive);
		sendReply(m_changeControlledTuningSourceReply);
	}

	void TcpTuningServer::onGetTuningServiceSettings(const char* requestData, quint32 requestDataSize)
	{
		Q_UNUSED(requestData)
		Q_UNUSED(requestDataSize)

		m_getServiceSettingsReply.Clear();

		m_getServiceSettingsReply.set_equipmentid(m_service.equipmentID().toStdString());
		m_getServiceSettingsReply.set_configip1(m_service.cfgServiceIP1().addressPortStr().toStdString());
		m_getServiceSettingsReply.set_configip2(m_service.cfgServiceIP2().addressPortStr().toStdString());

		sendReply(m_getServiceSettingsReply);
	}

	void TcpTuningServer::onGetTuningSourceFilling(const char* requestData, quint32 requestDataSize)
	{
		Q_UNUSED(requestData)
		Q_UNUSED(requestDataSize)

		m_getTuningSourceFillingReply.Clear();

		QList<quint64> sourceIDs = m_sourceId2SignalHash.uniqueKeys();

		m_getTuningSourceFillingReply.set_signalcount(m_sourceId2SignalHash.count());

		for (quint64 sourceID : sourceIDs)
		{
			Network::SignalsAssociatedToTuningSource* sourceMessage = m_getTuningSourceFillingReply.add_signalspersource();

			sourceMessage->set_sourceid(sourceID);

			for(const Hash signalHash : m_sourceId2SignalHash.values(sourceID))
			{
				sourceMessage->add_signalhash(signalHash);
			}
		}

		sendReply(m_getTuningSourceFillingReply);
	}

	void TcpTuningServer::onGetTuningSignalParam(const char* requestData, quint32 requestDataSize)
	{
		m_getAppSignalParamReply.Clear();

		bool result = m_getAppSignalParamRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
			sendReply(m_changeControlledTuningSourceReply);
			return;
		}

		int hashesCount = m_getAppSignalParamRequest.signalhashes_size();

		if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
		{
			m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
			sendReply(m_getAppSignalParamReply);
			return;
		}

		for (int i = 0; i < hashesCount; i++)
		{
			Hash signalHash = m_getAppSignalParamRequest.signalhashes(i);
			if (m_signalHash2SignalPtr.contains(signalHash) == false)
			{
				m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::UnknownSignalHash));
				sendReply(m_getAppSignalParamReply);
				return;
			}

			const AppSignal* signal = m_signalHash2SignalPtr.value(signalHash);

			if (signal == nullptr)
			{
				m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::UnknownSignalHash));
				sendReply(m_getAppSignalParamReply);
				return;
			}

			Proto::AppSignal* appSignalParam = m_getAppSignalParamReply.add_appsignals();

			signal->saveToProto(appSignalParam);
		}

		sendReply(m_getAppSignalParamReply);
	}

	void TcpTuningServer::onGetTuningSourcesFile(const char* requestData, quint32 requestDataSize)
	{
		Network::GetTuningSourcesFileRequest request;
		Network::GetTuningSourcesFileReply reply;

		bool result = request.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			reply.set_errcode(static_cast<qint32>(Tcp::FileTransferResult::RequestFormatError));
			sendReply(reply);
			return;
		}

		if (m_tuningSourcesFileData == nullptr ||
			m_tuningSourcesFileData->empty())
		{
			reply.set_errcode(static_cast<qint32>(Tcp::FileTransferResult::FileIsNotAccessible));
			sendReply(reply);
			return;
		}

		quint64 partNo = request.partno();
		quint64 fileSize = TO_QUINT64(m_tuningSourcesFileData->size());

		if (partNo * TDS_TUNING_SOURCES_FILE_PART_SIZE >= fileSize)
		{
			reply.set_errcode(static_cast<qint32>(Tcp::FileTransferResult::RequestFormatError));
			sendReply(reply);
			return;
		}

		//

		quint64 partsCount =
			(fileSize + TDS_TUNING_SOURCES_FILE_PART_SIZE - 1) / TDS_TUNING_SOURCES_FILE_PART_SIZE;

		quint64 partStart = partNo * TDS_TUNING_SOURCES_FILE_PART_SIZE;
		quint64 partSize = std::min(fileSize - partStart, TDS_TUNING_SOURCES_FILE_PART_SIZE);

		reply.set_errcode(static_cast<qint32>(Tcp::FileTransferResult::Ok));
		reply.set_filesize(m_tuningSourcesFileData->size());
		reply.set_partscount(partsCount);
		reply.set_filecrc64(m_tuningSourcesFileCrc64);
		reply.set_partno(partNo);
		reply.set_partsize(partSize);
		reply.set_filepartdata(m_tuningSourcesFileData->data() + partStart, partSize);

		sendReply(reply);
	}

	void TcpTuningServer::prepareSignalGetter()
	{
		for (const TuningSource& tuningSource : m_tuningSources)
		{
			TEST_PTR_CONTINUE(tuningSource.tuningData());

			QVector<AppSignal*> signalList;
			tuningSource.tuningData()->getSignals(&signalList);

			if (signalList.isEmpty() == true)
			{
				continue;
			}

			std::vector<quint32> IPs = tuningSource.lanControllersInfo().tuningIP32addresses();

			for(const AppSignal* signal : signalList)
			{
				TEST_PTR_CONTINUE(signal);

				Hash signalHash = calcHash(signal->appSignalID());

				m_signalHash2SignalPtr.insert(signalHash, signal);

				for(auto ip : IPs)
				{
					m_signalHash2SourceIP.insert(signalHash, ip);
				}

				m_sourceId2SignalHash.insert(tuningSource.ID(), signalHash);
			}
		}
	}

	void TcpTuningServer::initClientSourcesList(const QString& clientEquipmentID)
	{
		Q_ASSERT(m_clientSourcesList.has_value() == false);		// one time init

		if (clientEquipmentID == SCM_CLIENT_ID)
		{
			// send all sources info to SCM
			//
			m_clientSourcesList = m_tuningSources.getAllSourcesIDs();
		}
		else
		{
			auto settings = m_service.tuningServiceSettings();

			TuningServiceSettings::TuningClient tunClient = settings.getTuningClient(clientEquipmentID);

			DEBUG_LOG_MSG(m_logger, QString("Call initClientSourcesList for %1").arg(clientEquipmentID));

			if (tunClient.isValid() == false)
			{
				DEBUG_LOG_MSG(m_logger, QString("Client %1 not valid!").arg(clientEquipmentID));
				Q_ASSERT(false);		// clientEquipmentID should be checked early!
				return;
			}

			m_clientSourcesList = tunClient.uniqueSourcesIDs();

			DEBUG_LOG_MSG(m_logger, QString("Sources for %1: %2").arg(clientEquipmentID).arg(m_clientSourcesList.value().join(", ")));
		}
	}

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class implementation
	//
	// -------------------------------------------------------------------------------

	TcpTuningServerThread::TcpTuningServerThread(const HostAddressPort &listenAddress,
		E::SecurityLevel securityLevel,
		TcpTuningServer* server,
		std::shared_ptr<CircularLogger> logger) :
		Tcp::ListenerThread(listenAddress, securityLevel, server, logger, "TcpTuningServerThread")
	{
	}
}
