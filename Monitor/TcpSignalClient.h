#ifndef TCPSIGNALCLIENT_H
#define TCPSIGNALCLIENT_H

#include <QStringList>
#include "../lib/Tcp.h"
#include "../lib/Hash.h"
#include "../Proto/network.pb.h"
#include "../lib/AppSignalManager.h"
#include "MonitorConfigController.h"

//
//		ADS_GET_APP_SIGNAL_LIST_START
//				|
//		ADS_GET_APP_SIGNAL_LIST_NEXT
//				|
//		ADS_GET_APP_SIGNAL_PARAM
//				|
//		ADS_GET_UNITS
//				|
//		ADS_GET_APP_SIGNAL_STATE <------+
//				|						|			Repeat it
//				+------------------------
//

class TcpSignalClient : public Tcp::Client
{
	Q_OBJECT

public:
	TcpSignalClient(MonitorConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
	virtual ~TcpSignalClient();

protected:
	virtual void timerEvent(QTimerEvent* event) override;

public:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetSignalList();
	void resetToGetState();

	void requestSignalListStart();
	void processSignalListStart(const QByteArray& data);

	void requestSignalListNext(int part);
	void processSignalListNext(const QByteArray& data);

	void requestSignalParam(int startIndex);
	void processSignalParam(const QByteArray& data);

	void requestUnits();
	void processUnits(const QByteArray& data);

	void requestSignalState(int startIndex);
	void processSignalState(const QByteArray& data);

protected slots:
	void slot_configurationArrived(ConfigSettings configuration);

signals:
	void signalParamAndUnitsArrived();
	void connectionReset();

private:
	int m_startStateTimerId = -1;
	MonitorConfigController* m_cfgController = nullptr;

private:
	// Cache protobug messages
	//
	::Network::GetSignalListStartReply m_getSignalListStartReply;

	::Network::GetSignalListNextRequest m_getSignalListNextRequest;
	::Network::GetSignalListNextReply m_getSignalListNextReply;
	std::vector<QString> m_signalList;

	::Network::GetAppSignalParamRequest m_getSignalParamRequest;
	::Network::GetAppSignalParamReply m_getSignalParamReply;
	int m_lastSignalParamStartIndex = 0;

	::Network::GetAppSignalStateRequest m_getSignalStateRequest;
	::Network::GetAppSignalStateReply m_getSignalStateReply;
	int m_lastSignalStateStartIndex = 0;

	::Network::GetUnitsRequest m_getUnitsRequest;
	::Network::GetUnitsReply m_getUnitsReply;
};

#endif // TCPSIGNALCLIENT_H
