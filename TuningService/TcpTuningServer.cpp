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

		default:
			assert(false);
			break;
		}
	}


	void TcpTuningServer::onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesInfoReply.Clear();

		m_getTuningSourcesInfo.ParseFromArray(requestData, requestDataSize);

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

		for(const TuningClientContext* clientContext : clientContexts)
		{
			QVector<Network::DataSourceInfo> dsiArray;

			clientContext->getSourcesInfo(dsiArray);

			for(const Network::DataSourceInfo& dsi : dsiArray)
			{
				Network::DataSourceInfo* newDsi = m_getTuningSourcesInfoReply.add_tuningsourceinfo();

				if (newDsi == nullptr)
				{
					assert(false);
					continue;
				}

				*newDsi = dsi;
			}
		}

		errCode = NetworkError::Success;

		m_getTuningSourcesInfoReply.set_error(TO_INT(errCode));

		sendReply(m_getTuningSourcesInfoReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_GET_TUNING_SOURCES_INFO to %2")).
					  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
	}


	void TcpTuningServer::onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesStatesReply.Clear();

		m_getTuningSourcesStates.ParseFromArray(requestData, requestDataSize);

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

		for(const TuningClientContext* clientContext : clientContexts)
		{
			QVector<Network::TuningSourceState> tssArray;

			clientContext->getSourcesStates(tssArray);

			for(const Network::TuningSourceState& tss : tssArray)
			{
				Network::TuningSourceState* newTss = m_getTuningSourcesStatesReply.add_tuningsourcesstate();

				if (newTss == nullptr)
				{
					assert(false);
					continue;
				}

				*newTss = tss;
			}
		}

		m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesStatesReply);
	}


	void TcpTuningServer::onTuningSignalsReadRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsReadRequest.ParseFromArray(requestData, requestDataSize);

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientEquipmentID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_tuningSignalsReadReply.Clear();

			errCode = NetworkError::UnknownTuningClientID;

			m_tuningSignalsReadReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsReadReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_READ to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		// m_tuningSignalsReadReply.set_error(???) is set inside clientContext->readSignalStates()
		//
		clientContext->readSignalStates(m_tuningSignalsReadRequest, m_tuningSignalsReadReply);

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
		m_tuningSignalsWriteRequest.ParseFromArray(requestData, requestDataSize);

		QString clientEquipmentID = connectedSoftwareInfo().equipmentID();

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_TUNING_SIGNALS_WRITE request from client %1, %2 (Signals %3, AutoApply is %4)")).
					  arg(clientEquipmentID).
					  arg(peerAddr().addressStr()).
					  arg(m_tuningSignalsWriteRequest.commands_size()).
					  arg(m_tuningSignalsWriteRequest.autoapply() == true ? "TRUE" : "FALSE"));

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientEquipmentID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_tuningSignalsWriteReply.Clear();

			errCode = NetworkError::UnknownTuningClientID;

			m_tuningSignalsWriteReply.set_error(TO_INT(errCode));

			sendReply(m_tuningSignalsWriteReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_WRITE to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		// m_tuningSignalsWriteReply.set_error(???) is set inside clientContext->writeSignalStates()
		//
		clientContext->writeSignalStates(m_tuningSignalsWriteRequest, m_tuningSignalsWriteReply);

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


	void TcpTuningServer::onTuningSignalsApplyRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsApplyRequest.ParseFromArray(requestData, requestDataSize);

		QString clientRequestID = QString::fromStdString(m_tuningSignalsApplyRequest.clientequipmentid());

		DEBUG_LOG_MSG(m_logger, QString(tr("TDS_TUNING_SIGNALS_APPLY request from client %1, %2")).
					  arg(clientRequestID).
					  arg(peerAddr().addressStr()));

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientRequestID);

		NetworkError errCode = NetworkError::Success;

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_tuningSignalsApplyReply.Clear();

			m_tuningSignalsApplyReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));

			sendReply(m_tuningSignalsApplyReply);

			DEBUG_LOG_ERR(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
						  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
			return;
		}

		clientContext->applySignalStates();

		errCode = NetworkError::Success;

		m_tuningSignalsApplyReply.set_error(TO_INT(errCode));

		sendReply(m_tuningSignalsWriteReply);

		DEBUG_LOG_MSG(m_logger, QString(tr("Send reply %1 on TDS_TUNING_SIGNALS_APPLY to %2")).
					  arg(getNetworkErrorStr(errCode)).arg(peerAddr().addressStr()));
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
