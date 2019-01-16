#pragma once

#include "../lib/Tcp.h"

#include "../Proto/network.pb.h"
#include "../Proto/serialization.pb.h"
#include "../lib/TimeStamp.h"

#include "Archive.h"

class ArchRequest;

class TcpArchRequestsServer : public Tcp::Server
{
public:
	TcpArchRequestsServer(const SoftwareInfo& softwareInfo, Archive* archive, CircularLoggerShared logger);

	virtual Tcp::Server* getNewInstance() override;
	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

private:
	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	void onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize);
	void onGetSignalStatesFromArchiveNext(const char* requestData, quint32 requestDataSize);
	void onGetSignalStatesFromArchiveCancel(const char* requestData, quint32 requestDataSize);

	void finalizeCurrentRequest();

private:
	Archive* m_archive = nullptr;
	CircularLoggerShared m_logger;

	//

	ArchRequest* m_request = nullptr;
};
