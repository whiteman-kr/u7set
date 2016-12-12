#include "TcpTuningServer.h"

namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServer class implementation
	//
	// -------------------------------------------------------------------------------

	TcpTuningServer::TcpTuningServer(TuningServiceWorker& service) :
		m_service(service)
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
		TcpTuningServer* newServer =  new TcpTuningServer(m_service);

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

		default:
			assert(false);
			break;
		}
	}


	void TcpTuningServer::onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesInfoReply.Clear();

		m_getTuningSourcesInfo.ParseFromArray(requestData, requestDataSize);

		QString clientRequestID = QString::fromStdString(m_getTuningSourcesInfo.clientequipmentid());

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientRequestID);

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));
			sendReply(m_getTuningSourcesInfoReply);
			return;
		}

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

		m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesInfoReply);
	}


	void TcpTuningServer::onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesStatesReply.Clear();

		m_getTuningSourcesStates.ParseFromArray(requestData, requestDataSize);

		QString clientRequestID = QString::fromStdString(m_getTuningSourcesStates.clientequipmentid());

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientRequestID);

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));
			sendReply(m_getTuningSourcesStatesReply);
			return;
		}

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

		m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesStatesReply);
	}


	void TcpTuningServer::onTuningSignalsReadRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsReadRequest.ParseFromArray(requestData, requestDataSize);

		QString clientRequestID = QString::fromStdString(m_tuningSignalsReadRequest.clientequipmentid());

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientRequestID);

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_tuningSignalsReadReply.Clear();

			m_tuningSignalsReadReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));

			sendReply(m_tuningSignalsReadReply);
			return;
		}

		clientContext->readSignalStates(m_tuningSignalsReadRequest, m_tuningSignalsReadReply);

		// m_tuningSignalsReadReply.set_error(???) must be set inside clientContext->getSignalStates()

		sendReply(m_tuningSignalsReadReply);
	}


	void TcpTuningServer::onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize)
	{
		m_tuningSignalsWriteRequest.ParseFromArray(requestData, requestDataSize);

		QString clientRequestID = QString::fromStdString(m_tuningSignalsWriteRequest.clientequipmentid());

		const TuningClientContext* clientContext =
				m_service.getClientContext(clientRequestID);

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_tuningSignalsWriteReply.Clear();

			m_tuningSignalsWriteReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));

			sendReply(m_tuningSignalsWriteReply);
			return;
		}

		clientContext->writeSignalStates(m_tuningSignalsWriteRequest, m_tuningSignalsWriteReply);

		sendReply(m_tuningSignalsWriteReply);
	}



	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class implementation
	//
	// -------------------------------------------------------------------------------

	class TuningServiceWorker;

	TcpTuningServerThread::TcpTuningServerThread(const HostAddressPort& listenAddressPort,
							TcpTuningServer* server) :
		Tcp::ServerThread(listenAddressPort, server)
	{
	}

}
