#pragma once

#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../CommonLib/Hash.h"
#include "../Proto/network.pb.h"
#include "../Lib/SoftwareSettings.h"
#include "../AppSignalLib/AppSignalManager.h"
#include "MonitorConfigController.h"

class MonitorSignalManager;

//
//		ADS_GET_APP_SIGNAL_LIST_START
//				|
//		ADS_GET_APP_SIGNAL_LIST_NEXT
//				|
//		ADS_GET_APP_SIGNAL_PARAM
//				|
//		ADS_GET_APP_SIGNAL_STATE_CHANGES <----+
//              |                             |
//		ADS_GET_APP_SIGNAL_STATE              |
//				|						      |
//				+-----------------------------+
//
//
class TcpSignalClient : public Tcp::Client, public TcpClientStatistics, public HasLogFile
{
	Q_OBJECT

public:
	TcpSignalClient(const MonitorConfigController& configController,
					const MonitorSettings::AppDataService& adsInfo,
					MonitorSignalManager& signalManager,
					ILogFile* logFile);
	virtual ~TcpSignalClient();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	virtual Tcp::ConnectionState getConnectionState() const override;

	bool hasSignal(const QString& appSignalId) const;
	bool hasSignal(Hash signalHash) const;

protected:
	void resetToGetSignalList();
	void resetToGetState(bool resetStateIndex);

	void requestSignalListStart();
	void processSignalListStart(const QByteArray& data);

	void requestSignalListNext(int part);
	void processSignalListNext(const QByteArray& data);

	void requestSignalParam(int startIndex);
	void processSignalParam(const QByteArray& data);

	void requestSignalStateChanges();
	void processSignalStateChanges(const QByteArray& data);

	void requestSignalState(int startIndex);
	void processSignalState(const QByteArray& data);

signals:
	void connectionReset();

private:
	const MonitorConfigController& m_cfgController;		// Do we really need MonitorConfigController, or SoftwareInfo is just enough?
	MonitorSettings::AppDataService m_serverSettings;
	MonitorSignalManager& m_signalManager;

	// Keep own signal list, so MonitorSignalManager can understand if this connstion has a signal
	//
	mutable QReadWriteLock m_hasSignalLock;
	std::set<Hash> m_hasSignalList;			// Key is hash from signal internal id

	// Cache protobug messages
	//
private:
	::Network::GetSignalListStartReply m_getSignalListStartReply;

	::Network::GetSignalListNextRequest m_getSignalListNextRequest;
	::Network::GetSignalListNextReply m_getSignalListNextReply;
	std::vector<QString> m_signalList;

	::Network::GetAppSignalParamRequest m_getSignalParamRequest;
	::Network::GetAppSignalParamReply m_getSignalParamReply;
	int m_lastSignalParamStartIndex = 0;

	::Network::GetAppSignalStateChangesRequest m_getSignalStateChangesRequest;
	::Network::GetAppSignalStateChangesReply m_getSignalStateChangesReply;

	::Network::GetAppSignalStateRequest m_getSignalStateRequest;
	::Network::GetAppSignalStateReply m_getSignalStateReply;
	int m_lastSignalStateStartIndex = 0;
};

