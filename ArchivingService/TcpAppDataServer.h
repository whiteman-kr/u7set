#pragma once

#include "../OnlineLib/Tcp.h"
#include "Archive.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class declaration
//
// -------------------------------------------------------------------------------

class TcpAppDataServer : public Tcp::Server
{
public:
	TcpAppDataServer(const SoftwareInfo& softwareInfo,
					 Archive* archive);

	virtual Tcp::Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;
	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	void onSaveAppSignalsStatesToArchive(const char* requestData, quint32 requestDataSize);

private:
	Archive* m_archive = nullptr;

	Network::SaveAppSignalsStatesToArchiveRequest m_saveStatesRequest;
	Network::SaveAppSignalsStatesToArchiveReply m_saveStatesReply;
};

