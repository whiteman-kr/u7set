#include "TcpTuningServer.h"

namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServer class implementation
	//
	// -------------------------------------------------------------------------------

	const char* TcpTuningServer::SCM_CLIENT_ID = "SCM";

	TcpTuningServer::TcpTuningServer(TuningServiceWorker& service, std::shared_ptr<CircularLogger> logger) :
		Tcp::Server(service.softwareInfo()),
		m_service(service),
		m_logger(logger)
	{
	}

	void TcpTuningServer::onServerThreadStarted()
	{
	}

	void TcpTuningServer::onServerThreadFinished()
	{
	}

	void TcpTuningServer::onConnection()
	{
	}

	void TcpTuningServer::onDisconnection()
	{
		m_service.clientIsDisconnected(connectedSoftwareInfo(), peerAddr().addressStr());
	}

	Tcp::Server* TcpTuningServer::getNewInstance()
	{
		TcpTuningServer* newServer =  new TcpTuningServer(m_service, m_logger);

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

		case TDS_TUNING_SIGNALS_READ:
			onTuningSignalsReadRequest(requestData, requestDataSize);
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

		default:
			assert(false);
			break;
		}
	}

	void TcpTuningServer::onConnectedSoftwareInfoChanged()
	{
		m_service.clientIsConnected(connectedSoftwareInfo(), peerAddr().addressStr());
	}

	void TcpTuningServer::onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesInfoReply.Clear();

		bool result = m_getTuningSourcesInfo.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::ParseRequestError));
			sendReply(m_getTuningSourcesInfoReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_GET_TUNING_SOURCES_INFO request from %1, %2")).
					  arg(clientEquipmentID).arg(peerAddr().addressStr()));


		QVector<const TuningClientContext*> clientContexts;

		QString msg;

		NetworkError errCode = NetworkError::Success;

		if (clientEquipmentID == SCM_CLIENT_ID)
		{
			m_service.getAllClientContexts(clientContexts);
		}
		else
		{
			const TuningClientContext* clntContext =
					m_service.getClientContext(clientEquipmentID);

			if (clntContext == nullptr)
			{
				// unknown clientID
				//
				errCode = NetworkError::UnknownTuningClientID;

				m_getTuningSourcesInfoReply.set_error(TO_INT(errCode));
				sendReply(m_getTuningSourcesInfoReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_INFO to %2")).
							  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
				return;
			}

			clientContexts.append(clntContext);
		}

		QHash<quint64, quint64> dataSourcesIDs;

		for(const TuningClientContext* clientContext : clientContexts)
		{
			QVector<Network::DataSourceInfo> dsiArray;

			clientContext->getSourcesInfo(dsiArray);

			for(const Network::DataSourceInfo& dsi : dsiArray)
			{
				quint64 sourceID = dsi.id();

				if (dataSourcesIDs.contains(sourceID) == false)
				{
					dataSourcesIDs.insert(sourceID, sourceID);

					Network::DataSourceInfo* newDsi = m_getTuningSourcesInfoReply.add_tuningsourceinfo();

					if (newDsi == nullptr)
					{
						assert(false);
						continue;
					}

					*newDsi = dsi;
				}
			}
		}

		errCode = NetworkError::Success;

		m_getTuningSourcesInfoReply.set_singlelmcontrolmode(m_service.singleLmControl());
		m_getTuningSourcesInfoReply.set_activeclientid(m_service.activeClientID().toStdString());
		m_getTuningSourcesInfoReply.set_activeclientip(m_service.activeClientIP().toStdString());

		m_getTuningSourcesInfoReply.set_error(TO_INT(errCode));

		sendReply(m_getTuningSourcesInfoReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_INFO to %2")).
					  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
	}


	void TcpTuningServer::onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesStatesReply.Clear();

		bool result = m_getTuningSourcesStates.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::ParseRequestError));
			sendReply(m_getTuningSourcesStatesReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		QVector<const TuningClientContext*> clientContexts;

		if (clientEquipmentID == SCM_CLIENT_ID)
		{
			m_service.getAllClientContexts(clientContexts);
		}
		else
		{
			const TuningClientContext* clntContext =
					m_service.getClientContext(clientEquipmentID);

			if (clntContext == nullptr)
			{
				// unknown clientID
				//
				NetworkError errCode = NetworkError::UnknownTuningClientID;

				m_getTuningSourcesStatesReply.set_error(TO_INT(errCode));
				sendReply(m_getTuningSourcesStatesReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_STATES to %2")).
							  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
				return;
			}

			clientContexts.append(clntContext);
		}

		QHash<quint64, quint64> dataSourcesIDs;

		for(const TuningClientContext* clientContext : clientContexts)
		{
			QVector<Network::TuningSourceState> tssArray;

			clientContext->getSourcesStates(tssArray);

			for(const Network::TuningSourceState& tss : tssArray)
			{
				quint64 sourceID = tss.sourceid();

				if (dataSourcesIDs.contains(sourceID) == false)
				{
					dataSourcesIDs.insert(sourceID, sourceID);

					Network::TuningSourceState* newTss = m_getTuningSourcesStatesReply.add_tuningsourcesstate();

					if (newTss == nullptr)
					{
						assert(false);
						continue;
					}

					*newTss = tss;
				}
			}
		}

		m_getTuningSourcesStatesReply.set_singlelmcontrolmode(m_service.singleLmControl());
		m_getTuningSourcesStatesReply.set_activeclientid(m_service.activeClientID().toStdString());
		m_getTuningSourcesStatesReply.set_activeclientip(m_service.activeClientIP().toStdString());

		m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesStatesReply);
	}

	void TcpTuningServer::onTuningSignalsReadRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsReadReply.Clear();

		bool result = m_tuningSignalsReadRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_tuningSignalsReadReply.set_error(TO_INT(NetworkError::ParseRequestError));
			sendReply(m_tuningSignalsReadReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientEquipmentID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = NetworkError::UnknownTuningClientID;

			m_tuningSignalsReadReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsReadReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		// m_tuningSignalsReadReply.set_error(???) is set inside clientContext->readSignalStates()
		//
		clientContext->readSignalStates(m_tuningSignalsReadRequest, &m_tuningSignalsReadReply);

		sendReply(m_tuningSignalsReadReply);

		errCode = static_cast<NetworkError>(m_tuningSignalsReadReply.error());

		if (errCode != NetworkError::Success)
		{
			// log errors only
			//
			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
		}
	}

	void TcpTuningServer::onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsWriteReply.Clear();

		bool result = m_tuningSignalsWriteRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_tuningSignalsWriteReply.set_error(TO_INT(NetworkError::ParseRequestError));
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

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = NetworkError::UnknownTuningClientID;

			m_tuningSignalsWriteReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsWriteReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_service.singleLmControl() == true)
		{
			QString activeClientID = m_service.activeClientID();
			QString activeClientIP = m_service.activeClientIP();

			if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
			{
				errCode = NetworkError::ClientIsNotActive;

				m_tuningSignalsWriteReply.set_error(TO_INT(errCode));

				sendReply(m_tuningSignalsWriteReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
							  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
				return;
			}
		}

		QString user = connectedSoftwareInfo().userName();

		// m_tuningSignalsWriteReply.set_error(???) is set inside clientContext->writeSignalStates()
		//
		clientContext->writeSignalStates(clientEquipmentID, user, m_tuningSignalsWriteRequest, &m_tuningSignalsWriteReply);

		sendReply(m_tuningSignalsWriteReply);

		errCode = static_cast<NetworkError>(m_tuningSignalsWriteReply.error());

		QString msg = QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
				arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr());

		if (errCode == NetworkError::Success)
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
			m_tuningSignalsApplyReply.set_error(TO_INT(NetworkError::ParseRequestError));
			sendReply(m_tuningSignalsApplyReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_TUNING_SIGNALS_APPLY request from client %1, %2")).
					  arg(clientEquipmentID).
					  arg(peerAddr().addressStr()));

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientEquipmentID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			m_tuningSignalsApplyReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));

			sendReply(m_tuningSignalsApplyReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_service.singleLmControl() == true)
		{
			QString activeClientID = m_service.activeClientID();
			QString activeClientIP = m_service.activeClientIP();

			if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
			{
				errCode = NetworkError::ClientIsNotActive;

				m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

				sendReply(m_tuningSignalsApplyReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
							  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
				return;
			}
		}

		QString user = connectedSoftwareInfo().userName();

		clientContext->applySignalStates(clientEquipmentID, user);

		errCode = NetworkError::Success;

		m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

		sendReply(m_tuningSignalsApplyReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
					  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
	}

	void TcpTuningServer::onChangeControlledTuningSourceRequest(const char *requestData, quint32 requestDataSize)
	{
		m_changeControlledTuningSourceReply.Clear();

		bool result = m_changeControlledTuningSourceRequest.ParseFromArray(requestData, requestDataSize);

		if (result == false)
		{
			m_changeControlledTuningSourceReply.set_error(TO_INT(NetworkError::ParseRequestError));
			sendReply(m_changeControlledTuningSourceReply);
			return;
		}

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		const TuningClientContext* clientContext = m_service.getClientContext(clientEquipmentID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			errCode = NetworkError::UnknownTuningClientID;

			m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

			sendReply(m_changeControlledTuningSourceReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		if (m_service.singleLmControl() == false)
		{
			errCode = NetworkError::SingleLmControlDisabled;

			m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

			sendReply(m_changeControlledTuningSourceReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		QString activeClientID = m_service.activeClientID();
		QString activeClientIP = m_service.activeClientIP();

		if (clientEquipmentID != activeClientID || peerAddr().addressStr() != activeClientIP)
		{
			if (m_changeControlledTuningSourceRequest.takecontrol() == false)
			{
				errCode = NetworkError::ClientIsNotActive;

				m_changeControlledTuningSourceReply.set_error(TO_INT(errCode));

				sendReply(m_changeControlledTuningSourceReply);

				DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_CHANGE_CONTROLLED_TUNING_SOURCE to %2")).
							  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
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

		NetworkError errorCode = m_service.changeControlledTuningSource(tuningSourceEquipmentID,
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
		m_getServiceSettingsReply.set_configip1(m_service.cfgServiceIP1().toStdString());
		m_getServiceSettingsReply.set_configip2(m_service.cfgServiceIP2().toStdString());

		sendReply(m_getServiceSettingsReply);
	}


	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class implementation
	//
	// -------------------------------------------------------------------------------

	class TuningServiceWorker;

	TcpTuningServerThread::TcpTuningServerThread(const HostAddressPort& listenAddressPort,
							TcpTuningServer* server,
							std::shared_ptr<CircularLogger> logger) :
		Tcp::ServerThread(listenAddressPort, server, logger)
	{
	}

}
