#pragma once

#include "../OnlineLib/Tcp.h"
#include "TuningSource.h"

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
		TcpTuningServer(TuningServiceWorker& service,
						const TuningSources& tuningSources,
						std::shared_ptr<CircularLogger> logger);
	private:
		virtual void onServerThreadStarted() override;
		virtual void onServerThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		Tcp::Server* getNewInstance(const Tcp::ListenAddress& listenAddr) override;

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

		virtual void onConnectedSoftwareInfoChanged() override;

		void onGetTuningSourcesInfoRequest(const char* requestData, quint32 requestDataSize);
		void onGetTuningSourcesStateRequest(const char* requestData, quint32 requestDataSize);
		void onTuningSignalsReadRequest(const char* requestData, quint32 requestDataSize);
		void onGetTuningSignalsStateChangesRequest(const char* requestData, quint32 requestDataSize);
		void onTuningSignalsWriteRequest(const char* requestData, quint32 requestDataSize);
		void onTuningSignalsApplyRequest(const char* requestData, quint32 requestDataSize);
		void onChangeControlledTuningSourceRequest(const char* requestData, quint32 requestDataSize);
		void onGetTuningServiceSettings(const char* requestData, quint32 requestDataSize);

		void onGetTuningSourceFilling(const char* requestData, quint32 requestDataSize);
		void onGetTuningSignalParam(const char* requestData, quint32 requestDataSize);

		void prepareSignalGetter();

		void initClientSourcesList(const QString& clientEquipmentID);

	private:
		TuningServiceWorker& m_service;

		const TuningSources& m_tuningSources;

		//

		static const QString SCM_CLIENT_ID;

		static quint64 m_staticTcpConnectionID;

		QThread* m_thread = nullptr;

		QString m_clientEquipmentID;
		quint64 m_tcpConnectionID = 0;

		//

		QHash<Hash, const AppSignal*> m_signalHash2SignalPtr;
		QHash<Hash, quint32> m_signalHash2SourceIP;
		QMultiHash<quint64, Hash> m_sourceId2SignalHash;

		std::optional<QStringList> m_clientSourcesList;

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

		Network::GetTuningSignalsStateChangesReply m_getStateChangesReply;
	};

	// -------------------------------------------------------------------------------
	//
	// TcpTuningServerThread class declaration
	//
	// -------------------------------------------------------------------------------

	class TcpTuningServerThread : public Tcp::ListenerThread
	{
	public:
		TcpTuningServerThread(const HostAddressPort& listenAddress,
							  E::SecurityLevel securityLevel,
							  TcpTuningServer* server,
							  std::shared_ptr<CircularLogger> logger);
	};

}
