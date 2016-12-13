#pragma once

#include "../lib/Tcp.h"
#include "../Proto/network.pb.h"
#include "TuningSource.h"
#include "TuningService.h"

namespace Tuning
{

	class TuningServiceWorker;

	// -------------------------------------------------------------------------------
	//
	// TcpTuningDataServer class declaration
	//
	// -------------------------------------------------------------------------------

	class TcpTuningServer  : public Tcp::Server
	{
	public:
		TcpTuningServer(TuningServiceWorker& service);

//		void setThread(TcpTuningServerThread* thread);

	private:
		virtual void onServerThreadStarted() override;
		virtual void onServerThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		Tcp::Server* getNewInstance() override;

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

		void onGetTuningSourcesInfoRequest(const char* requestData, quint32 requestDataSize);
		void onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsReadRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsApplyRequest(const char *requestData, quint32 requestDataSize);

	private:
		static const char* SCM_CLIENT_ID;

		TuningServiceWorker& m_service;

		Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
		Network::GetTuningSourcesInfoReply m_getTuningSourcesInfoReply;

		Network::GetTuningSourcesStates m_getTuningSourcesStates;
		Network::GetTuningSourcesStatesReply m_getTuningSourcesStatesReply;

		Network::TuningSignalsRead m_tuningSignalsReadRequest;
		Network::TuningSignalsReadReply m_tuningSignalsReadReply;

		Network::TuningSignalsWrite m_tuningSignalsWriteRequest;
		Network::TuningSignalsWriteReply m_tuningSignalsWriteReply;

		Network::TuningSignalsApply m_tuningSignalsApplyRequest;
		Network::TuningSignalsApplyReply m_tuningSignalsApplyReply;
	};


	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class declaration
	//
	// -------------------------------------------------------------------------------

	class TcpTuningServerThread : public Tcp::ServerThread
	{
	public:
		TcpTuningServerThread(const HostAddressPort& listenAddressPort,
								TcpTuningServer* server);
	};

}
