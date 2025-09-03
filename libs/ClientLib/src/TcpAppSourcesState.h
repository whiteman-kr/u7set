#pragma once

#include <map>
#include <vector>

#include <QReadWriteLock>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include <Network.pb.h>

#include <ClientLib/AppDataSourceState.h>

namespace ClientLib
{
	//
	//	ADS_GET_APP_DATA_SOURCES_INFO
	//				|
	//	ADS_GET_APP_DATA_SOURCES_STATES <------+
	//				|						   |	Repeat it
	//				+--------------------------+
	//
	class TcpAppSourcesState : public Tcp::Client,
							   public TcpClientStatistics
	{
		Q_OBJECT

	public:
		TcpAppSourcesState(const SoftwareInfo& softwareInfo, const SoftwareEndpoint::AppDataService& ads, ILogFile* logFile);
		virtual ~TcpAppSourcesState();

		std::vector<ClientLib::AppDataSourceState> appDataSourceStates() const;

		int sourceErrorCount();

	public:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	protected:
		void resetToGetAppDataSourcesInfo();
		void resetToGetAppDataSourcesState();

		void requestAppDataSourcesInfo();
		void processAppDataSourcesInfo(const QByteArray& data);

		void requestAppDataSourcesState();
		void processAppDataSourcesState(const QByteArray& data);

	private:
		HasLogFile m_logFile;

	private:
		int m_requestPeriod = 100;

		mutable QReadWriteLock m_appDataSourceStatesLock;            // For access to m_appDataSourceStates
		std::map<quint64, AppDataSourceState> m_appDataSourceStates; // Key is source unique id

		// Cache protobuf messages
		//
		::Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
		::Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStateReply;
	};

} // namespace ClientLib
