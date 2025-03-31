#pragma once

#include "../OnlineLib/Tcp.h"

class Service;

class TcpSrvInfoServer : public Tcp::Server
{
public:
	TcpSrvInfoServer(const SoftwareInfo& sotwareInfo,
					 const QString& serverDescription,
					 Service& service);

	virtual ~TcpSrvInfoServer();

	virtual Tcp::Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	void onServiceGetInfo();

private:
	Service& m_service;
};
