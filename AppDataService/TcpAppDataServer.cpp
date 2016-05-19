#include "TcpAppDataServer.h"

TcpAppDataServer::TcpAppDataServer()
{

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
