#include <QString>
#include <QtTest>
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
		m_udpSockets.push_back(udpSocket);

		bool result = udpSocket->bind(source.lmAddress(), source.lmPort());
		if (result == false)
		{
			error = "Unnable to bind to " + source.lmAddressPort().addressPortStr();
			return false;
		}
	}

	return true;
}

bool TestAppDataService::initTcpClient(QString &/*error*/)
{
	removeTcpClient();

	SoftwareInfo si;

	si.init(E::SoftwareType::TestClient, m_equipmentID, 1, 0);

	m_tcpClientSocket = new AppDataServiceClient(m_nextDataSourceStateMessage, si, m_cfgSettings.appDataService_clientRequestIP);

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
	delete m_tcpClientSocket;
	m_tcpClientSocket = nullptr;
}

bool TestAppDataService::sendBuffers(QString& error, bool checkHeaderErrors)
{
	// Getting all counters before sending any udp
	//
	if (m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_DATA_SOURCES_STATES, error) == false)
	{
		error = "Got error while getting AppDataSource state: " + error;
		return false;
	}

	// Checking correctness of initialization
	//
	if (m_dataSources.size() != m_sourcePackets.size())
	{
		assert(false);
		error = "Data source quantity not equal packet quantity";
		return false;
	}

	// Sending buffers (frames)
	//
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

	// Getting all counters after sending udp
	//
	m_previousDataSourceStateMessage = m_nextDataSourceStateMessage;
	if (m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_DATA_SOURCES_STATES, error) == false)
	{
		error = "Got error while getting AppDataSource state: " + error;
		return false;
	}

	// Checking correctness of all data quantity counters
	//
	for (int i = 0; i < m_dataSources.size(); i++)
	{
		// Get AppDataSource state messages to compare
		//
		auto* previusSourceMessage = getSourceMessageById(m_previousDataSourceStateMessage, m_dataSources[i].ID());
		auto* nextSourceMessage = getSourceMessageById(m_nextDataSourceStateMessage, m_dataSources[i].ID());

		if (previusSourceMessage == nullptr || nextSourceMessage == nullptr)
		{
			error = "AppDataSource state reply doesn't contain state of " + m_dataSources[i].lmEquipmentID();
			return false;
		}

		// AppDataSourceState::receivedDataID
		//
		Rup::Header& header = m_sourcePackets[i][0].header;
		quint32 dataId = reverseUint32(header.dataId);
		if (dataId != nextSourceMessage->receiveddataid())
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received Data ID " + QString::number(nextSourceMessage->receiveddataid(), 16) + " instead of " + QString::number(dataId, 16);
			return false;
		}

		// AppDataSourceState::receivedPacketCount
		//
		int receivedPacketsQuantity = nextSourceMessage->receivedpacketcount() - previusSourceMessage->receivedpacketcount();

		if (receivedPacketsQuantity != 1)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received " + QString::number(receivedPacketsQuantity) + " packets instead of 1";
			return false;
		}

		// AppDataSourceState::receivedFramesCount
		//
		int receivedFramesQuantity = nextSourceMessage->receivedframescount() - previusSourceMessage->receivedframescount();

		if (receivedFramesQuantity != m_dataSources[i].lmRupFramesQuantity())
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received " + QString::number(receivedFramesQuantity) + " frames"
					" instead of " + QString::number(m_dataSources[i].lmRupFramesQuantity());
			return false;
		}

		// AppDataSourceState::receivedDataSize
		//
		int receivedDataQuantity = nextSourceMessage->receiveddatasize() - previusSourceMessage->receiveddatasize();
		int expectedDataQuantity = m_dataSources[i].lmRupFramesQuantity() * Socket::ENTIRE_UDP_SIZE;

		if (receivedDataQuantity != expectedDataQuantity)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received " + QString::number(receivedFramesQuantity) + "bytes"
					" instead of " + QString::number(expectedDataQuantity);
			return false;
		}

		// AppDataSourceState::lostedPacketCount
		//
		int lostPacketCount = nextSourceMessage->lostedpacketcount() - previusSourceMessage->lostedpacketcount();

		if (lostPacketCount != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" lost " + QString::number(lostPacketCount) + " packets instead of 0";
			return false;
		}

		// AppDataSourceState::rupFramePlantTime
		//
		Rup::TimeStamp timeStamp = header.timeStamp;
		timeStamp.reverseBytes();

		QDateTime expectedPlantTime;

		expectedPlantTime.setTimeSpec(Qt::UTC);	// don't delete this to prevent plantTime conversion from Local to UTC time!!!

		expectedPlantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
		expectedPlantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

		QDateTime receivedPlantTime = QDateTime::fromMSecsSinceEpoch(nextSourceMessage->rupframeplanttime());

		if (receivedPlantTime != expectedPlantTime)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received plant time " + receivedPlantTime.toString() +
					" instead of " + expectedPlantTime.toString();
			return false;
		}

		// AppDataSourceState::rupFrameNumerator
		//
		quint16 numerator = reverseUint16(header.numerator);
		if (numerator != nextSourceMessage->rupframenumerator())
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" received numerator " + QString::number(nextSourceMessage->rupframenumerator()) +
					" instead of " + QString::number(numerator);
			return false;
		}

		if (checkHeaderErrors == false)
		{
			continue;
		}

		// AppDataSourceState::errorProtocolVersion
		//
		int errorProtocolVersion = nextSourceMessage->errorprotocolversion() - previusSourceMessage->errorprotocolversion();
		if (errorProtocolVersion != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorProtocolVersion by " + errorProtocolVersion;
			return false;
		}

		// AppDataSourceState::errorFramesQuantity
		//
		int errorFramesQuantity = nextSourceMessage->errorframesquantity() - previusSourceMessage->errorframesquantity();
		if (errorFramesQuantity != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorFramesQuantity by " + errorFramesQuantity;
			return false;
		}

		// AppDataSourceState::errorFrameNo
		//
		int errorFrameNo = nextSourceMessage->errorframeno() - previusSourceMessage->errorframeno();
		if (errorFrameNo != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorFrameNo by " + errorFrameNo;
			return false;
		}

		// AppDataSourceState::errorDataID
		//
		int errorDataID = nextSourceMessage->errordataid() - previusSourceMessage->errordataid();
		if (errorDataID != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorDataID by " + errorDataID;
			return false;
		}

		// AppDataSourceState::errorFrameSize
		//
		int errorFrameSize = nextSourceMessage->errorframesize() - previusSourceMessage->errorframesize();
		if (errorFrameSize != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorFrameSize by " + errorFrameSize;
			return false;
		}

		// AppDataSourceState::errorDuplicatePlantTime
		//
		int errorDuplicatePlantTime = nextSourceMessage->errorduplicateplanttime() - previusSourceMessage->errorduplicateplanttime();
		if (errorDuplicatePlantTime != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorDuplicatePlantTime by " + errorDuplicatePlantTime;
			return false;
		}

		// AppDataSourceState::errorNonmonotonicPlantTime
		//
		int errorNonmonotonicPlantTime = nextSourceMessage->errornonmonotonicplanttime() - previusSourceMessage->errornonmonotonicplanttime();
		if (errorNonmonotonicPlantTime != 0)
		{
			error = "Source " + m_dataSources[i].lmEquipmentID() +
					" incrimented errorNonmonotonicPlantTime by " + errorNonmonotonicPlantTime;
			return false;
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
			[&](const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
	{
		data = configurationXmlData;
		waiter.quit();
	});
	connect(&timeoutChecker, &QTimer::timeout, &waiter, &QEventLoop::quit);
	timeoutChecker.start(10 * 1000);
	waiter.exec();

	if (timeoutChecker.isActive() == false)
	{
		QVERIFY2(false, qPrintable("Timeout of reading Configuration.xml"));
	}

	XmlReadHelper configXml(data);

	bool result = m_cfgSettings.readFromXml(configXml);
	if (result == false)
	{
		QVERIFY2(false, qPrintable("Could not understand config file Configuration.xml"));
	}

	result = initTcpClient(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	result = m_tcpClientSocket->sendRequestAndWaitForResponse(ADS_GET_DATA_SOURCES_INFO, error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
	}

	m_tcpClientSocket->initDataSourceArray(m_dataSources);

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
}

void TestAppDataService::TADS_001_001()
{
	QString error;

	// Sending udp and checking transmission counters
	//
	bool result = sendBuffers(error);
	if (result == false)
	{
		QVERIFY2(false, qPrintable(error));
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
