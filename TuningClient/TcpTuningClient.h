#ifndef TCPTUNINGCLIENT_H
#define TCPTUNINGCLIENT_H

#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "ObjectManager.h"
#include "ConfigController.h"

class TcpTuningClient : public Tcp::Client
{
	Q_OBJECT
public:
	TcpTuningClient(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	~TcpTuningClient();

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetTuningSources();
	//void resetToGetState();

	void requestTuningSourcesStart();
	void processTuningSourcesStart(const QByteArray& data);

	/*void requestSignalListNext(int part);
	void processSignalListNext(const QByteArray& data);

	void requestSignalParam(int startIndex);
	void processSignalParam(const QByteArray& data);

	void requestUnits();
	void processUnits(const QByteArray& data);

	void requestSignalState(int startIndex);
	void processSignalState(const QByteArray& data);*/

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void tuningSourcesArrived();
	void connectionFailed();


private:
	ConfigController* m_cfgController = nullptr;

private:
	// Cache protobug messages
	//
	::Network::TuningDataSourceState m_getSignalListStartReply;

	::Network::GetSignalListNextRequest m_getSignalListNextRequest;
	::Network::GetSignalListNextReply m_getSignalListNextReply;
	std::vector<QString> m_signalList;
};

#endif // TCPTUNINGCLIENT_H
