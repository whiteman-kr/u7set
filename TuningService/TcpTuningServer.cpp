#include "TcpTuningServer.h"

namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServer class implementation
	//
	// -------------------------------------------------------------------------------

	TcpTuningServer::TcpTuningServer()
	{

	}


	void TcpTuningServer::setThread(TcpTuningServerThread* thread)
	{
		assert(thread != nullptr);
		m_thread = thread;
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
		TcpTuningServer* newServer =  new TcpTuningServer();

		newServer->setThread(m_thread);

		return newServer;
	}


	void TcpTuningServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		switch(requestID)
		{
		case TDS_GET_TUNING_SOURCES_INFO:
			onGetTuningSourcesInfoRequest();
			break;

		case TDS_GET_TUNING_SOURCES_STATES:
			onGetTuningSourcesStateRequest();
			break;

		default:
			assert(false);
			break;
		}
	}


	void TcpTuningServer::onGetTuningSourcesInfoRequest()
	{
		m_getTuningSourcesInfoReply.Clear();

		const TuningDataSources& tuningSrcs = tuningSources();

		for(const TuningDataSource* source : tuningSrcs)
		{
			Network::DataSourceInfo* protoInfo = m_getTuningSourcesInfoReply.add_datasourceinfo();
			source->getInfo(protoInfo);
		}

		m_getTuningSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

		sendReply(m_getTuningSourcesInfoReply);
	}


	void TcpTuningServer::onGetTuningSourcesStateRequest()
	{
	}


	TuningDataSources& TcpTuningServer::tuningSources()
	{
		return m_thread->tuningSources();
	}


	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class implementation
	//
	// -------------------------------------------------------------------------------

	TcpTuningServerThread::TcpTuningServerThread(const HostAddressPort& listenAddressPort,
							TcpTuningServer* server,
							TuningDataSources& tuningSources) :
		Tcp::ServerThread(listenAddressPort, server),
		m_tuningSources(tuningSources)
	{
		server->setThread(this);
	}


	TuningDataSources& TcpTuningServerThread::tuningSources()
	{
		return 	m_tuningSources;
	}

}
