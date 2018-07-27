#include <QString>
#include <QtTest>
#include <QUdpSocket>
#include "tst_testappdataservice.h"
#include "../../lib/XmlHelper.h"
#include "AppDataServiceClient.h"
#include "../../lib/SoftwareInfo.h"

const QString pathToBuild = QStringLiteral("/mnt/Data/fedora/service_tests-debug");
const QString appDataServiceID = QStringLiteral("SYSTEMID_RACKID_WS00_ADS");

const QString configFile = QStringLiteral("Configuration.xml");
const QString dataSourceFile = QStringLiteral("AppDataSources.xml");

const QString appDataServiceBuildPath = pathToBuild + (pathToBuild.endsWith("/") ? "" : "/") +
		"build/" +
		appDataServiceID + "/";

const QString appDataServiceConfigPath = appDataServiceBuildPath + configFile;
const QString appDataServiceDataSourcePath = appDataServiceBuildPath + dataSourceFile;

TestAppDataService::TestAppDataService()
{
}

bool TestAppDataService::readByteArray(QString fileName, QByteArray& result, QString& error)
{
	if (QFile::exists(fileName) == false)
	{
		error = "File " + fileName + "doesn't exist";
		return false;
	}

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		error = "Could not open " + fileName;
		return false;
	}

	result = file.readAll();
	return true;
}

bool TestAppDataService::initSourcePackets(QString& error)
{
	m_sourcePackets.clear();

	for (const DataSource& source : m_dataSources)
	{
		QVector<Rup::Frame> sourceFrames;

		if (source.lmRupFramesQuantity() == 0)
		{
			error = "Source with " + source.lmAddressPort().addressPortStr() + " has zero frames";
			return false;
		}

		for (int i = 0; i < source.lmRupFramesQuantity(); i++)
		{
			Rup::Frame frame;

			Rup::Header& rh = frame.header;

			rh.frameSize = Socket::ENTIRE_UDP_SIZE;
			rh.protocolVersion = Rup::VERSION;
			rh.flags.all = 0;
			rh.flags.appData = 1;
			rh.dataId = source.lmDataID();
			rh.moduleType = source.lmModuleType();
			rh.numerator = 0;

			rh.framesQuantity = source.lmRupFramesQuantity();
			rh.frameNumber = i;

			QDateTime t = QDateTime::currentDateTime();

			rh.timeStamp.year = t.date().year();
			rh.timeStamp.month = t.date().month();
			rh.timeStamp.day = t.date().day();

			rh.timeStamp.hour = t.time().hour();
			rh.timeStamp.minute = t.time().minute();
			rh.timeStamp.second = t.time().second();
			rh.timeStamp.millisecond = t.time().msec();

			rh.reverseBytes();

			memset(frame.data, 0, sizeof(frame.data));

			frame.calcCRC64();

			sourceFrames.push_back(frame);
		}

		m_sourcePackets.push_back(sourceFrames);
	}

	return true;
}

bool TestAppDataService::initSenders(QString &error)
{
	removeSenders();

	for (const DataSource& source : m_dataSources)
	{
		QUdpSocket* udpSocket = new QUdpSocket(this);

		bool result = udpSocket->bind(source.lmAddress(), source.lmPort());
		if (result == false)
		{
			error = "Unnable to bind to " + source.lmAddressPort().addressPortStr();
			return false;
		}

		m_udpSockets.push_back(udpSocket);
	}

	return true;
}

bool TestAppDataService::initTcpClient(QString &/*error*/)
{
	removeTcpClient();

	SoftwareInfo si;

	si.init(E::SoftwareType::ServiceControlManager, "TEST_APP_DATA_SERVICE", 1, 0);

	m_tcpClientSocket = new AppDataServiceClient(m_nextDataSourceStateMessage, si, m_cfgSettings.clientRequestIP);

	return true;
}

void TestAppDataService::removeSenders()
{
	for (QUdpSocket* socket : m_udpSockets)
	{
		delete socket;
	}
	m_udpSockets.clear();
}

void TestAppDataService::removeTcpClient()
{
	disconnect(tcpClientSendRequestConnection);

	if (m_tcpClientThread == nullptr)
	{
		return;
	}
	m_tcpClientThread->quitAndWait();
	delete m_tcpClientThread;
	m_tcpClientThread = nullptr;

	m_tcpClientSocket = nullptr;
}

bool TestAppDataService::sendBuffers(QString& error)
{
	if (m_dataSources.size() != m_sourcePackets.size())
	{
		assert(false);
		error = "Data source quantity not equal packet quantity";
		return false;
	}

	for (int i = 0; i < m_dataSources.size(); i++)
	{
		const DataSource& source = m_dataSources[i];
		QVector<Rup::Frame>& sourceFrames = m_sourcePackets[i];

		if (source.lmRupFramesQuantity() != sourceFrames.size())
		{
			assert(false);
			error = "Wrong frame quantity";
			return false;
		}

		// Increment numerator
		quint16 numerator = reverseUint16(sourceFrames[source.lmRupFramesQuantity() - 1].header.numerator);
		numerator++;

		for (int j = 0; j < source.lmRupFramesQuantity(); j++)
		{
			Rup::Frame& frame = sourceFrames[j];

			frame.header.numerator = reverseUint16(numerator);

			const HostAddressPort& dest = m_cfgSettings.appDataReceivingIP;
			m_udpSockets[i]->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(frame), dest.address(), dest.port());
		}
	}

	return true;
}

const Network::AppDataSourceState* TestAppDataService::getSourceMessageById(Network::GetAppDataSourcesStatesReply &reply, quint64 id)
{
	for (int i = 0; i < reply.appdatasourcesstates_size(); i++)
	{
		const Network::AppDataSourceState& sourceMessage = reply.appdatasourcesstates(i);
		if (sourceMessage.id() == id)
		{
			return &sourceMessage;
		}
	}
	return nullptr;
}

void TestAppDataService::initTestCase()
{
	// reading Configuration.xml
	//
	QByteArray data;
	QString error;
	bool result = readByteArray(appDataServiceConfigPath, data, error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}
	XmlReadHelper configXml(data);

	result = m_cfgSettings.readFromXml(configXml);
	if (result == false)
	{
		QVERIFY2(false, qPrintable("Could not read config file " + appDataServiceConfigPath));
	}

	// reading AppDataSources.xml
	//
	result = readByteArray(appDataServiceDataSourcePath, data, error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	result = DataSourcesXML<DataSource>::readFromXml(data, &m_dataSources);
	if (result == false)
	{
		QVERIFY2(false, qPrintable("Error reading AppDataSources from XML-file " + appDataServiceConfigPath));
	}

	result = initSourcePackets(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	result = initSenders(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	result = initTcpClient(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}
}

void TestAppDataService::TADS_001_001()
{
	QString error;

	// Getting all counters before sending any udp
	//
	if (m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_DATA_SOURCES_STATES, error) == false)
	{
		QVERIFY2(false, qPrintable("Got error while getting AppDataSource state: " + error));
	}

	// Sending udp
	//
	bool result = sendBuffers(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	// Getting all counters after sending udp
	//
	m_previousDataSourceStateMessage = m_nextDataSourceStateMessage;
	if (m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_DATA_SOURCES_STATES, error) == false)
	{
		QVERIFY2(false, qPrintable("Got error while getting AppDataSource state: " + error));
	}

	// Checking correctness of all data quantity counters
	//
	for (int i = 0; i < m_dataSources.size(); i++)
	{
		auto* previusSourceMessage = getSourceMessageById(m_previousDataSourceStateMessage, m_dataSources[i].ID());
		auto* nextSourceMessage = getSourceMessageById(m_nextDataSourceStateMessage, m_dataSources[i].ID());

		if (previusSourceMessage == nullptr || nextSourceMessage == nullptr)
		{
			QVERIFY2(false, qPrintable("Source state reply doesn't containes state of " + m_dataSources[i].lmCaption()));
		}

		int receivedPacketsQuantity = nextSourceMessage->receivedpacketcount() - previusSourceMessage->receivedpacketcount();

		if (receivedPacketsQuantity != 1)
		{
			QVERIFY2(false, qPrintable("Source " + m_dataSources[i].lmCaption() +
									   " received " + QString::number(receivedPacketsQuantity) + " packets instead of 1"));
		}

		int receivedFramesQuantity = nextSourceMessage->receivedframescount() - previusSourceMessage->receivedframescount();

		if (receivedFramesQuantity != m_dataSources[i].lmRupFramesQuantity())
		{
			QVERIFY2(false, qPrintable("Source " + m_dataSources[i].lmCaption() +
									   " received " + QString::number(receivedFramesQuantity) + " frames"
									   " instead of " + QString::number(m_dataSources[i].lmRupFramesQuantity())));
		}

		int receivedDataQuantity = nextSourceMessage->receiveddatasize() - previusSourceMessage->receiveddatasize();
		int expectedDataQuantity = m_dataSources[i].lmRupFramesQuantity() * Socket::ENTIRE_UDP_SIZE;

		if (receivedDataQuantity != expectedDataQuantity)
		{
			QVERIFY2(false, qPrintable("Source " + m_dataSources[i].lmCaption() +
									   " received " + QString::number(receivedFramesQuantity) + "bytes"
									   " instead of " + QString::number(expectedDataQuantity)));
		}
	}
}

void TestAppDataService::cleanupTestCase()
{
	removeSenders();
	removeTcpClient();
}

//QTEST_APPLESS_MAIN(TestAppDataService)
//QTEST_GUILESS_MAIN(TestAppDataService)
QTEST_MAIN(TestAppDataService)

//#include "tst_testappdataservice.moc"
