#ifndef TST_TESTAPPDATASERVICE_H
#define TST_TESTAPPDATASERVICE_H
#include <QObject>
#include "../../lib/ServiceSettings.h"
#include "../../lib/DataSource.h"

class AppDataServiceClient;
class QUdpSocket;

class TestAppDataService : public QObject
{
	Q_OBJECT

signals:
	void sendRequest(quint32 requestID);

public:
	TestAppDataService(int& argc, char** argv);

private:
	void readByteArray(QString fileName, QByteArray& resultBuffer, bool& result);
	void initSourcePackets(bool& result);
	void initSenders(bool& result);
	void initTcpClient();
	void removeSenders();
	void removeTcpClient();
	void sendBuffers(bool& result);

	const Network::AppDataSourceState *getSourceMessageById(Network::GetAppDataSourcesStatesReply& reply, quint64 id);

private Q_SLOTS:
	void initTestCase();

	void TADS_001_001();	// Send correct data
	void TADS_002_001();	// TADS_002_* Initiate frame header errors

	void cleanupTestCase();

private:
	QString m_equipmentID;
	HostAddressPort m_cfgIp1;
	HostAddressPort m_cfgIp2;
	TestClientSettings m_cfgSettings;
	QVector<DataSource> m_dataSources;
	QVector<QVector<Rup::Frame>> m_sourcePackets;

	AppDataServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

	QVector<QUdpSocket*> m_udpSockets;

	Network::GetAppDataSourcesStatesReply m_previousDataSourceStateMessage;
	Network::GetAppDataSourcesStatesReply m_nextDataSourceStateMessage;
};

#endif // TST_TESTAPPDATASERVICE_H
