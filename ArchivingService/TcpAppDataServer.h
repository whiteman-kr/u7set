#pragma once

#include "../OnlineLib/Tcp.h"
#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "Archive.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
public:
	TcpAppDataServer(const SoftwareInfo& softwareInfo, Archive* archive);

	virtual Tcp::Server* getNewInstance() override;
	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	void onSaveAppSignalsStatesToArchive(const char* requestData, quint32 requestDataSize);

	void onConnection() override;
	void onDisconnection() override;

private:
	Archive* m_archive = nullptr;

	Network::SaveAppSignalsStatesToArchiveRequest m_saveStatesRequest;
	Network::SaveAppSignalsStatesToArchiveReply m_saveStatesReply;
};

