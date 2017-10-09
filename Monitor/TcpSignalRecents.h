#ifndef TCPSIGNALRECENTS_H
#define TCPSIGNALRECENTS_H

#include <QStringList>
#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "../lib/AppSignalManager.h"
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

	int size() const;
	const std::map<Hash, qint64>& rawHashes() const;	// Just faster access to map
	std::vector<Hash> hashes() const;

private:
	int m_maxSize = 750;
	std::map<Hash, qint64> m_signalToTile;				// first - signal hash, second - time of last update
	std::multimap<qint64, Hash> m_timeToSignal;			// second - time of last update, first - signal hash
};



class TcpSignalRecents : public Tcp::Client
{
	Q_OBJECT

public:
	TcpSignalRecents(MonitorConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
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

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

private:
	MonitorConfigController* m_cfgController = nullptr;

	QMutex m_mutex;			// Mutex for access to m_recents, really it's not required if addSignal used as a slot
	RecentUsed m_recents = RecentUsed(ADS_GET_APP_SIGNAL_STATE_MAX);

private:
	// Cache protobug messages
	//
	::Network::GetAppSignalStateRequest m_getSignalStateRequest;
	::Network::GetAppSignalStateReply m_getSignalStateReply;
};



#endif // TCPSIGNALRECENTS_H
