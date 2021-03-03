#include <QString>
#include "TestUtils.h"
#include <QUdpSocket>
#include "TestAppDataService.h"
#include "../../lib/XmlHelper.h"
#include "AppDataServiceClient.h"
#include "../../lib/SoftwareInfo.h"
#include "../../lib/CommandLineParser.h"
#include "../../lib/CfgServerLoader.h"

const QString configFile = QStringLiteral("Configuration.xml");
const QString dataSourceFile = QStringLiteral("AppDataSources.xml");

const QString SETTING_EQUIPMENT_ID = QStringLiteral("EquipmentID");
const QString SETTING_CFG_SERVICE_IP1 = QStringLiteral("CfgServiceIP1");
const QString SETTING_CFG_SERVICE_IP2 = QStringLiteral("CfgServiceIP2");

TestAppDataService::TestAppDataService(int &argc, char **argv)
{
	CommandLineParser parser(argc, argv);

	parser.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "TestClient EquipmentID.", "EQUIPMENT_ID");
	parser.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	parser.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");

	parser.parse();

	m_equipmentID = parser.settingValue(SETTING_EQUIPMENT_ID);
	m_cfgIp1.setAddressPortStr(parser.settingValue(SETTING_CFG_SERVICE_IP1), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgIp2.setAddressPortStr(parser.settingValue(SETTING_CFG_SERVICE_IP2), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
}

void TestAppDataService::readByteArray(QString fileName, QByteArray& resultBuffer, bool& result)
{
	VERIFY_STATEMENT_STR(QFile::exists(fileName), "File " + fileName + "doesn't exist");

	QFile file(fileName);

	VERIFY_STATEMENT_STR(file.open(QIODevice::ReadOnly | QIODevice::Text), "Could not open " + fileName);

	resultBuffer = file.readAll();
}

void TestAppDataService::initSourcePackets(bool& result)
{
	m_sourcePackets.clear();

	for (const DataSource& source : m_dataSources)
	{
		QVector<Rup::Frame> sourceFrames;

		VERIFY_STATEMENT_STR(source.lmRupFramesQuantity() > 0, "Source with " + source.lmAddressPort().addressPortStr() + " has zero frames");

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
}

void TestAppDataService::initSenders(bool& result)
{
	removeSenders();

	for (const DataSource& source : m_dataSources)
	{
		QUdpSocket* udpSocket = new QUdpSocket(this);
		m_udpSockets.push_back(udpSocket);

		VERIFY_STATEMENT_STR(udpSocket->bind(source.lmAddress(), source.lmPort()), "Unnable to bind to " + source.lmAddressPort().addressPortStr());
	}
}

void TestAppDataService::initTcpClient()
{
	removeTcpClient();

	SoftwareInfo si;

	si.init(E::SoftwareType::TestClient, m_equipmentID, 1, 0);

	m_tcpClientSocket = new AppDataServiceClient(m_nextDataSourceStateMessage, si, m_cfgSettings.appDataService_clientRequestIP);
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
	delete m_tcpClientSocket;
	m_tcpClientSocket = nullptr;
}

void TestAppDataService::sendBuffers(bool& result)
{
	// Sending buffers (frames)
	//
	for (int i = 0; i < m_dataSources.size(); i++)
	{
		const DataSource& source = m_dataSources[i];
		QVector<Rup::Frame>& sourceFrames = m_sourcePackets[i];

		VERIFY_STATEMENT(source.lmRupFramesQuantity() == sourceFrames.size(), "Wrong frame quantity");

		// Increment numerator
		quint16 numerator = reverseUint16(sourceFrames[source.lmRupFramesQuantity() - 1].header.numerator);
		numerator++;

		for (int j = 0; j < source.lmRupFramesQuantity(); j++)
		{
			Rup::Frame& frame = sourceFrames[j];

			Rup::Header& rh = frame.header;

			rh.numerator = reverseUint16(numerator);

			QDateTime t = QDateTime::currentDateTime();

			Rup::TimeStamp& rts = rh.timeStamp;

			rts.year = t.date().year();
			rts.month = t.date().month();
			rts.day = t.date().day();

			rts.hour = t.time().hour();
			rts.minute = t.time().minute();
			rts.second = t.time().second();
			rts.millisecond = t.time().msec();

			rts.reverseBytes();

			//static int attemptQuantity = 0;

			const HostAddressPort& dest = m_cfgSettings.appDataService_appDataReceivingIP;
			m_udpSockets[i]->writeDatagram(reinterpret_cast<char*>(&frame), sizeof(frame), dest.address(), dest.port());

			/*attemptQuantity++;
			if (attemptQuantity > 1)
			{
				error = "Sent more than one frame";
				return false;
			}*/
		}
	}
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
	bool result = true;

	// reading Configuration.xml
	//
	SoftwareInfo si;

	si.init(E::SoftwareType::TestClient, m_equipmentID, 1, 0);

	CfgLoaderThread cfgLoader(si, 1, m_cfgIp1, m_cfgIp2, false, nullptr);

	cfgLoader.start();
	cfgLoader.enableDownloadConfiguration();

	QTimer timeoutChecker;
	QEventLoop waiter;

	QByteArray data;
	QString error;

	timeoutChecker.setSingleShot(true);
	connect(&cfgLoader, &CfgLoaderThread::signal_configurationReady,
			[&](const QByteArray configurationXmlData, const BuildFileInfoArray /*buildFileInfoArray*/)
	{
		data = configurationXmlData;
		waiter.quit();
	});

	connect(&timeoutChecker, &QTimer::timeout, &waiter, &QEventLoop::quit);
	timeoutChecker.start(10 * 1000);
	waiter.exec();

	QVERIFY2(timeoutChecker.isActive(), "Timeout of reading Configuration.xml");

	std::shared_ptr<const TestClientSettings> curSettingsPtr = cfgLoader.getCurrentSettingsProfile<TestClientSettings>();

	QVERIFY2(curSettingsPtr != nullptr, "Error getting current settings profile!");

	m_cfgSettings = *curSettingsPtr.get();

	initTcpClient();

	m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_APP_DATA_SOURCES_INFO, result);
	VERIFY_RESULT("Got error while requesting ADS_GET_APP_DATA_SOURCES_INFO");

	m_tcpClientSocket->initDataSourceArray(m_dataSources);

	initSourcePackets(result);
	VERIFY_RESULT("Got error while initializing fake AppDataSources");

	initSenders(result);
	VERIFY_RESULT("Got error while initializing udp senders");
}

void TestAppDataService::TADS_001_001()
{
	bool result = true;

	// Checking correctness of initialization
	//
	QVERIFY2(m_dataSources.size() == m_sourcePackets.size(), "Data source quantity not equal packet quantity");

	// Getting all counters before sending any udp
	//
	m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_APP_DATA_SOURCES_STATES, result);
	VERIFY_RESULT("Got error while getting AppDataSource state");

	// Sending udp and checking transmission counters
	//
	sendBuffers(result);
	VERIFY_RESULT("Could not send registration data");

	// Getting all counters after sending udp
	//
	m_previousDataSourceStateMessage = m_nextDataSourceStateMessage;

	m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_APP_DATA_SOURCES_STATES, result);
	VERIFY_RESULT("Got error while getting AppDataSource state");

	// Checking correctness of all data quantity counters
	//
	for (int i = 0; i < m_dataSources.size(); i++)
	{
		result = false;
		// Get AppDataSource state messages to compare
		//
		auto* previusSourceMessage = getSourceMessageById(m_previousDataSourceStateMessage, m_dataSources[i].ID());
		auto* nextSourceMessage = getSourceMessageById(m_nextDataSourceStateMessage, m_dataSources[i].ID());

		VERIFY_STR(previusSourceMessage != nullptr && nextSourceMessage != nullptr,
				   "AppDataSource state reply doesn't contain state of " + m_dataSources[i].lmEquipmentID());

		// AppDataSourceState::receivedDataID
		//
		Rup::Header& header = m_sourcePackets[i][0].header;
		quint32 dataId = reverseUint32(header.dataId);

		VERIFY_STR(dataId == nextSourceMessage->receiveddataid(),
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received Data ID " + QString::number(nextSourceMessage->receiveddataid(), 16) +
				   " instead of " + QString::number(dataId, 16));

		// AppDataSourceState::receivedPacketCount
		//
		qint64 receivedPacketsQuantity = nextSourceMessage->receivedpacketcount() - previusSourceMessage->receivedpacketcount();

		VERIFY_STR(receivedPacketsQuantity == 1,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received " + QString::number(receivedPacketsQuantity) +
				   " packets instead of 1");

		// AppDataSourceState::receivedFramesCount
		//
		qint64 receivedFramesQuantity = nextSourceMessage->receivedframescount() - previusSourceMessage->receivedframescount();

		VERIFY_STR(receivedFramesQuantity == m_dataSources[i].lmRupFramesQuantity(),
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received " + QString::number(receivedFramesQuantity) +
				   " frames instead of " + QString::number(m_dataSources[i].lmRupFramesQuantity()));

		// AppDataSourceState::receivedDataSize
		//
		qint64 receivedDataQuantity = nextSourceMessage->receiveddatasize() - previusSourceMessage->receiveddatasize();
		int expectedDataQuantity = m_dataSources[i].lmRupFramesQuantity() * Socket::ENTIRE_UDP_SIZE;

		VERIFY_STR(receivedDataQuantity == expectedDataQuantity,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received " + QString::number(receivedFramesQuantity) +
				   " bytes instead of " + QString::number(expectedDataQuantity));

		// AppDataSourceState::lostedPacketCount
		//
		qint64 lostPacketCount = nextSourceMessage->lostpacketcount() - previusSourceMessage->lostpacketcount();

		VERIFY_STR(lostPacketCount == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " lost " + QString::number(lostPacketCount) +
				   " packets instead of 0");

		// AppDataSourceState::rupFramePlantTime
		//
		Rup::TimeStamp timeStamp = header.timeStamp;
		timeStamp.reverseBytes();

		QDateTime expectedPlantTime;

		expectedPlantTime.setTimeSpec(Qt::UTC);	// don't delete this to prevent plantTime conversion from Local to UTC time!!!

		expectedPlantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
		expectedPlantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

		QDateTime receivedPlantTime = QDateTime::fromMSecsSinceEpoch(nextSourceMessage->rupframeplanttime());

		VERIFY_STR(receivedPlantTime == expectedPlantTime,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received plant time " + receivedPlantTime.toString() +
				   " instead of " + expectedPlantTime.toString());

		// AppDataSourceState::rupFrameNumerator
		//
		quint16 numerator = reverseUint16(header.numerator);
		VERIFY_STR(numerator != nextSourceMessage->rupframenumerator(),
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " received numerator " + QString::number(nextSourceMessage->rupframenumerator()) +
				   " instead of " + QString::number(numerator));

		/*
		 *  Verify error counters
		 */

		// AppDataSourceState::errorProtocolVersion
		//
		qint64 errorProtocolVersion = nextSourceMessage->errorprotocolversion() - previusSourceMessage->errorprotocolversion();
		VERIFY_STR(errorProtocolVersion == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorProtocolVersion by " + QString::number(errorProtocolVersion));

		// AppDataSourceState::errorFramesQuantity
		//
		qint64 errorFramesQuantity = nextSourceMessage->errorframesquantity() - previusSourceMessage->errorframesquantity();
		VERIFY_STR(errorFramesQuantity == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorFramesQuantity by " + QString::number(errorFramesQuantity));

		// AppDataSourceState::errorFrameNo
		//
		qint64 errorFrameNo = nextSourceMessage->errorframeno() - previusSourceMessage->errorframeno();
		VERIFY_STR(errorFrameNo == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorFrameNo by " + QString::number(errorFrameNo));

		// AppDataSourceState::errorDataID
		//
		qint64 errorDataID = nextSourceMessage->errordataid() - previusSourceMessage->errordataid();
		VERIFY_STR(errorDataID == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorDataID by " + QString::number(errorDataID));

		// AppDataSourceState::errorFrameSize
		//
		qint64 errorFrameSize = nextSourceMessage->errorframesize() - previusSourceMessage->errorframesize();
		VERIFY_STR(errorFrameSize == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorFrameSize by " + QString::number(errorFrameSize));

		// AppDataSourceState::errorDuplicatePlantTime
		//
		qint64 errorDuplicatePlantTime = nextSourceMessage->errorduplicateplanttime() - previusSourceMessage->errorduplicateplanttime();
		VERIFY_STR(errorDuplicatePlantTime == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorDuplicatePlantTime by " + QString::number(errorDuplicatePlantTime));

		// AppDataSourceState::errorNonmonotonicPlantTime
		//
		qint64 errorNonmonotonicPlantTime = nextSourceMessage->errornonmonotonicplanttime() - previusSourceMessage->errornonmonotonicplanttime();
		VERIFY_STR(errorNonmonotonicPlantTime == 0,
				   "Source " + m_dataSources[i].lmEquipmentID() +
				   " incrimented errorNonmonotonicPlantTime by " + QString::number(errorNonmonotonicPlantTime));
	}
}

void TestAppDataService::TADS_002_001()
{

}

void TestAppDataService::cleanupTestCase()
{
	removeSenders();
	removeTcpClient();
}
