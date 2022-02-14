#ifndef TCPSIGNALRECENTS_H
#define TCPSIGNALRECENTS_H

#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../CommonLib/Hash.h"
#include "../Proto/network.pb.h"
#include "MonitorSignalManager.h"
#include "MonitorConfigController.h"


//		ADS_GET_APP_SIGNAL_STATE <------+
//				|						|			Repeat it
//				+------------------------
//

class RecentUsed
{
public:
	explicit RecentUsed(int maxSize = 750);

public:
	void add(Hash h);
	void add(const QVector<Hash>& hashes);

	bool remove(Hash hash);
	bool remove(const std::vector<Hash>& hashes);

	int size() const;
	const std::map<Hash, qint64>& rawHashes() const;	// Just faster access to map
	std::vector<Hash> hashes() const;

private:
	int m_maxSize = 750;
	std::map<Hash, qint64> m_signalToTime;				// first - signal hash, second - time of last update
	std::multimap<qint64, Hash> m_timeToSignal;			// second - time of last update, first - signal hash
};



class TcpSignalRecents : public Tcp::Client, public TcpClientStatistics, public HasLogFile
{
	Q_OBJECT

public:
	TcpSignalRecents(const MonitorConfigController& configController,
					 const MonitorSettings::AppDataService& adsInfo,
					 MonitorSignalManager& signalManager,
					 ILogFile* logFile);
	virtual ~TcpSignalRecents();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;
	virtual void onConnection() override;
	virtual void onDisconnection() override;
	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

public slots:
	void addSignal(Hash hash);
	void addSignals(QVector<Hash> hashes);

protected:
	void requestSignalState();
	void processSignalState(const QByteArray& data);

signals:
	void connectionReset();

private:
	const MonitorConfigController& m_cfgController;
	MonitorSettings::AppDataService m_serverSettings;
	MonitorSignalManager& m_signalManager;

	RecentUsed m_recents = RecentUsed(ADS_GET_APP_SIGNAL_STATE_MAX);

private:
	// Cache protobug messages
	//
	::Network::GetAppSignalStateRequest m_getSignalStateRequest;
	::Network::GetAppSignalStateReply m_getSignalStateReply;
};



#endif // TCPSIGNALRECENTS_H
