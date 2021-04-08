#pragma once

#include "../OnlineLib/Tcp.h"
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
		TcpTuningServer(TuningServiceWorker& service, TuningSources& tuningSources, std::shared_ptr<CircularLogger> logger);

//		void setThread(TcpTuningServerThread* thread);

	private:
		virtual void onServerThreadStarted() override;
		virtual void onServerThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		Tcp::Server* getNewInstance() override;

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

		virtual void onConnectedSoftwareInfoChanged() override;

		void onGetTuningSourcesInfoRequest(const char* requestData, quint32 requestDataSize);
		void onGetTuningSourcesStateRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsReadRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsWriteRequest(const char *requestData, quint32 requestDataSize);
		void onTuningSignalsApplyRequest(const char *requestData, quint32 requestDataSize);
		void onChangeControlledTuningSourceRequest(const char *requestData, quint32 requestDataSize);
		void onGetTuningServiceSettings(const char *requestData, quint32 requestDataSize);

		void onGetTuningSourceFilling(const char *requestData, quint32 requestDataSize);
		void onGetTuningSignalParam(const char *requestData, quint32 requestDataSize);

		void prepareSignalGetter();

	private:
		static const char* SCM_CLIENT_ID;

		TuningServiceWorker& m_service;

		TuningSources& m_tuningSources;

		QHash<Hash, const Signal*> m_signalHash2SignalPtr;
		QHash<Hash, quint32> m_signalHash2SourceIP;
		QMultiHash<quint64, Hash> m_sourceId2SignalHash;

		std::shared_ptr<CircularLogger> m_logger;

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

		Network::ChangeConrolledTuningSourceRequest m_changeControlledTuningSourceRequest;
		Network::ChangeConrolledTuningSourceReply m_changeControlledTuningSourceReply;

		Network::TuningSourceFilling m_getTuningSourceFillingReply;

		Network::GetAppSignalParamRequest m_getAppSignalParamRequest;
		Network::GetAppSignalParamReply m_getAppSignalParamReply;

		Network::ServiceSettings m_getServiceSettingsReply;
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
								TcpTuningServer* server, std::shared_ptr<CircularLogger> logger);
	};

}
