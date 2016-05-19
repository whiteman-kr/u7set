#pragma once

#include "../include/Tcp.h"

class TcpAppDataServer : public Tcp::Server
{
private:
	virtual Server* getNewInstance() { return new TcpAppDataServer(); }

public:
	TcpAppDataServer();

	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;
};

