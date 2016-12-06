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

		default:
			assert(false);
			break;
		}
	}


	void TcpTuningServer::onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesInfoReply.Clear();

		m_getTuningSourcesInfo.ParseFromArray(requestData, requestDataSize);

		const TuningClientContext* clientContext =
				m_service.getClientContext(m_getTuningSourcesInfo.clientequipmentid());

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
			*newDsi = dsi;
		}

		m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesInfoReply);
	}


	void TcpTuningServer::onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize)
	{
		m_getTuningSourcesStatesReply.Clear();

		m_getTuningSourcesStates.ParseFromArray(requestData, requestDataSize);

		const TuningClientContext* clientContext =
				m_service.getClientContext(m_getTuningSourcesStates.clientequipmentid());

		if (clientContext == nullptr)
		{
			// unknown clientID
			//
			m_getTuningSourcesStatesReply.set_error(TO_INT(NetworkError::UnknownTuningClientID));
			sendReply(m_getTuningSourcesStatesReply);
			return;
		}

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
