#pragma once

#include "SignalBase.h"

#include "../../lib/Tcp.h"

// -------------------------------------------------------------------------------
//
// TcpTuningServer class declaration
//
// -------------------------------------------------------------------------------

class UalTesterServer  : public Tcp::Server
{
	Q_OBJECT

public:
	UalTesterServer(const SoftwareInfo& sotwareInfo, SignalBase* signalBase);

private:
	virtual void onServerThreadStarted() override;
	virtual void onServerThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;


	Tcp::Server* getNewInstance() override;

	virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	void onGetTuningSourcesInfoRequest(const char *requestData, quint32 requestDataSize);
	void onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize);
	void onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize);

private:

	SoftwareInfo m_sotwareInfo;
	SignalBase* m_signalBase = nullptr;

	Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
	Network::GetTuningSourcesInfoReply m_getTuningSourcesInfoReply;

	Network::GetTuningSourcesStates m_getTuningSourcesStates;
	Network::GetTuningSourcesStatesReply m_getTuningSourcesStatesReply;

	Network::TuningSignalsWrite m_tuningSignalsWriteRequest;
	Network::TuningSignalsWriteReply m_tuningSignalsWriteReply;

private	slots:

	void clientConnectionChanged(bool isConnect);
	void clientSignalStateChanged(Hash hash, double prevState, double state);

signals:

	void connectionChanged(bool isConnect);
	void signalStateChanged(Hash hash, double prevState, double state);
};


// -------------------------------------------------------------------------------
//
// TcpTuningServerThread class declaration
//
// -------------------------------------------------------------------------------

class UalTesterServerThread : public Tcp::ServerThread
{

public:
	UalTesterServerThread(const HostAddressPort& listenAddressPort, UalTesterServer* server, std::shared_ptr<CircularLogger> logger);
};
