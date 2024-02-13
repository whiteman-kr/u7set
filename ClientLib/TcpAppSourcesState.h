#pragma once

#include <vector>
#include <map>

#include <QReadWriteLock>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../Proto/Network.pb.h"


namespace ClientLib
{
	class AppDataSourceState
	{
	public:
		AppDataSourceState();

		quint64 id() const;
		QString equipmentId() const;

		void setNewState(const ::Network::AppDataSourceState& newState);

		int getErrorsCount() const;

		bool valid() const;
		void invalidate();

		const ::Network::AppDataSourceState& previousState() const;

	public:
		::Network::DataSourceInfo info;
		::Network::AppDataSourceState state;

	private:
		qint64 m_previousStateUpdatePeriod = 5;

		bool m_valid = true;

		::Network::AppDataSourceState m_previousState;	// Previous state is updated every 5 seconds

		QDateTime m_perviousStateLastUpdateTime;
	};

	//
	//	ADS_GET_APP_DATA_SOURCES_INFO
	//				|
	//	ADS_GET_APP_DATA_SOURCES_STATES <------+
	//				|						   |	Repeat it
	//				+--------------------------+
	//
	class TcpAppSourcesState : public Tcp::Client, public TcpClientStatistics
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

		mutable QReadWriteLock m_appDataSourceStatesLock;	// For access to m_appDataSourceStates
		std::map<quint64, AppDataSourceState> m_appDataSourceStates;	// Key is source unique id

		// Cache protobuf messages
		//
		::Network::GetDataSourcesInfoReply m_getDataSourcesInfoReply;
		::Network::GetAppDataSourcesStatesReply m_getAppDataSourcesStateReply;
	};

}
