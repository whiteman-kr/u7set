#pragma once

#include "../lib/Tcp.h"

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"

#include "../lib/AppSignal.h"


// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
public:
	TcpAppDataServer(AppSignalStatesQueue& saveStatesQueue);

	virtual Tcp::Server* getNewInstance() override;
	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	void onSaveAppSignalsStatesToArchive(const char* requestData, quint32 requestDataSize);

private:
	Network::SaveAppSignalsStatesToArchiveRequest m_saveStatesRequest;
	Network::SaveAppSignalsStatesToArchiveReply m_saveStatesReply;

	AppSignalStatesQueue& m_saveStatesQueue;
};


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServerThread : public Tcp::ServerThread
{
public:
	TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
				 Tcp::Server* server,
				 std::shared_ptr<CircularLogger> logger);
};
