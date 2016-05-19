#include "TcpAppDataServer.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer()
{

}


TcpAppDataServer::~TcpAppDataServer()
{
}


Tcp::Server* TcpAppDataServer::getNewInstance()
{
	TcpAppDataServer* newServer =  new TcpAppDataServer();

	newServer->setThread(m_thread);

	return newSever;
}


void TcpAppDataServer::onServerThreadStarted()
{
	qDebug() << "TcpAppDataServer::onServerThreadStarted()";
}


void TcpAppDataServer::onServerThreadFinished()
{
	qDebug() << "TcpAppDataServer::onServerThreadFinished()";
}


void TcpAppDataServer::onConnection()
{
	qDebug() << "TcpAppDataServer::onConnection()";
}


void TcpAppDataServer::onDisconnection()
{
	qDebug() << "TcpAppDataServer::onDisconnection()";
}


void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	int a = 0;
}


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(	const HostAddressPort& listenAddressPort,
												TcpAppDataServer* server,
												const AppSignals &appSignals) :
	Tcp::ServerThread(listenAddressPort, server),
	m_appSignals(appSignals)
{
	server->setThread(this);
}


void TcpAppDataServerThread::buildAppSignalIDs()
{
	m_appSignalIDs.clear();

	m_appSignalIDs.resize(m_appSignals.count());

	int i = 0;

	for(Signal* signal : m_appSignals)
	{
		m_appSignalIDs[i] = signal->appSignalID();

		i++;
	}
}
