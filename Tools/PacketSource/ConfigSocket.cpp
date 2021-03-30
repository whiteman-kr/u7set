#include "ConfigSocket.h"

#include <assert.h>
#include <QtConcurrent>
#include <QTimer>

#include "../../lib/SoftwareSettings.h"
#include "../../lib/DataSource.h"

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(	const SoftwareInfo& softwareInfo,
							const HostAddressPort& serverAddressPort,
							PacketSourceCore* pscore)
	: m_softwareInfo(softwareInfo)
	, m_serverAddressPort1(serverAddressPort)
	, m_serverAddressPort2(HostAddressPort(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST))
	, m_pscore(pscore)
{
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::ConfigSocket(	const SoftwareInfo& softwareInfo,
							const HostAddressPort& serverAddressPort1,
							const HostAddressPort& serverAddressPort2,
							PacketSourceCore* pscore)
	: m_softwareInfo(softwareInfo)
	, m_serverAddressPort1(serverAddressPort1)
	, m_serverAddressPort2(serverAddressPort2)
	, m_pscore(pscore)
{
}

// -------------------------------------------------------------------------------------------------------------------

ConfigSocket::~ConfigSocket()
{
	quit();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::clearConfiguration()
{
	if (m_pscore == nullptr)
	{
		return;
	}

	// save states of sources and signals, perhaps it reload
	//
	m_pscore->saveSourceState();

	// clear bases
	//
	m_pscore->clearAllBases();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::start()
{
	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											1,
											m_serverAddressPort1,
											m_serverAddressPort2,
											false,
											nullptr);

	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);
		return;
	}

	m_appDataSrvEquipmentID.clear();
	m_appDataSrvIP.clear();
	m_loadedFiles.clear();

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);
	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_unknownClient, this, &ConfigSocket::unknownClient);

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	startConnectionStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::quit()
{
	stopConnectionStateTimer();

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	disconnect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigSocket::slot_configurationReady);

	m_cfgLoaderThread->quitAndWait();
	delete m_cfgLoaderThread;
	m_cfgLoaderThread = nullptr;

	m_connected = false;
	m_cfgSrvIP.clear();

	emit socketDisconnected();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::reconncect()
{
	if (m_pscore == nullptr)
	{
		assert(m_pscore);
		return;
	}

	m_softwareInfo.setEquipmentID(m_pscore->buildOption().cfgSrvEquipmentID());
	m_serverAddressPort1 = m_pscore->buildOption().cfgSrvIP();
	m_serverAddressPort2 = HostAddressPort(QString("127.0.0.1"), PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	quit();
	start();
}

// -------------------------------------------------------------------------------------------------------------------

QString ConfigSocket::cfgSrvInfo()
{
	if (m_pscore == nullptr)
	{
		return QString();
	}

	QString connectedState;

	HostAddressPort configSocketAddress1 = cfgSrvIP();

	connectedState.append(tr("Equipment ID: %1\n").arg(m_softwareInfo.equipmentID()));

	if (m_cfgSrvIP.isEmpty() == true)
	{
		connectedState.append(tr("Not connected\n\n"));
	}
	else
	{
		connectedState.append(tr("Connected: %1 : %2\n\n").arg(m_cfgSrvIP.addressStr()).arg(m_cfgSrvIP.port()));
	}

	connectedState.append(tr("Loaded sources: %1\n").arg(m_pscore->sourceBase().count()));
	connectedState.append(tr("Loaded signals: %1\n\n").arg(m_pscore->signalBase().count()));

	int filesCount = m_loadedFiles.count();

	connectedState.append(tr("Loaded files: %1").arg(filesCount));

	for(int f = 0; f < filesCount; f++)
	{
		connectedState.append("\n" + m_loadedFiles.at(f));
	}

	return connectedState;
}

// -------------------------------------------------------------------------------------------------------------------

QString ConfigSocket::appDataSrvInfo()
{
	QString connectedState;

	if (m_appDataSrvEquipmentID.isEmpty() == true)
	{
		connectedState.append(tr("Equipment ID: Not loaded\n"));
	}
	else
	{
		connectedState.append(tr("Equipment ID: %1\n").arg(m_appDataSrvEquipmentID));
	}

	if (m_appDataSrvIP.isEmpty() == true)
	{
		connectedState.append(tr("Packets send to: Not loaded"));
	}
	else
	{
		connectedState.append(tr("Packets send to: %1 : %2").arg(m_appDataSrvIP.addressStr()).arg(m_appDataSrvIP.port()));
	}

	return connectedState;
}


// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::slot_configurationReady(const QByteArray configurationXmlData,
										   const BuildFileInfoArray buildFileInfoArray,
										   SessionParams sessionParams,
										   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	Q_UNUSED(sessionParams)

	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	m_loadedFiles.clear();
	clearConfiguration();

	bool result = false;

	result = readConfiguration(configurationXmlData, curSettingsProfile);
	if (result == false)
	{
		return;
	}

	for(const Builder::BuildFileInfo& bfi : buildFileInfoArray)
	{
		QByteArray fileData;
		QString errStr;

		m_cfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

		if (errStr.isEmpty() == false)
		{
			qDebug() << errStr;
			continue;
		}

		result = true;

		if (bfi.ID == CfgFileId::APP_SIGNAL_SET)
		{
			result &= readAppSignalSet(fileData);						// fill AppSignalSet
		}

		if (bfi.ID == CfgFileId::APP_DATA_SOURCES)
		{
			QString adsID = bfi.pathFileName;
			adsID.remove(File::APP_DATA_SOURCES_XML);
			adsID.remove('/');

			if (adsID != m_appDataSrvEquipmentID)
			{
				continue;
			}

			result &= readAppDataSource(fileData);						// fill AppDataSource
		}

		m_loadedFiles.append(bfi.pathFileName);
	}

	m_softwareRunMode = sessionParams.softwareRunMode;

	qDebug() << "Read configuration - Time for read:" << responseTime.elapsed() << "ms" << result;

	emit configurationLoaded();

	QtConcurrent::run(ConfigSocket::loadSignalBase, this);

	return;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readConfiguration(const QByteArray& fileData,
									 std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	if (m_pscore == nullptr)
	{
		assert(m_pscore);
		return false;
	}

	m_appDataSrvEquipmentID.clear();
	m_appDataSrvIP.clear();

	XmlReadHelper xml(fileData);

	if (xml.findElement(XmlElement::APP_DATA_SERVICES) == false)
	{
		qDebug() << __FUNCTION__ << "Error:" <<XmlElement::APP_DATA_SERVICES << "- section not found";
		return false;
	}

	int adsIDcount = 0;
	xml.readIntAttribute(XmlAttribute::COUNT, &adsIDcount);
	if (adsIDcount == 0)
	{
		qDebug() << __FUNCTION__ << "Error: Count of EquipmentIDs of AppDataSrv == 0";
		return false;
	}

	QString adsID;
	xml.readStringAttribute(XmlAttribute::ID, &adsID);

	QStringList adsIDList = adsID.split(';');
	if (adsIDList.count() != adsIDcount)
	{
		qDebug() << __FUNCTION__ << "Error: Wrong count of EquipmentIDs of AppDataSrv";
		return false;
	}

	int index = adsIDList.indexOf(m_pscore->buildOption().appDataSrvEquipmentID());
	if (index == -1)
	{
		emit unknownAdsEquipmentID(adsIDList);
		return false;
	}

	m_appDataSrvEquipmentID = adsIDList[index];

	const TestClientSettings* typedSettingsPtr = dynamic_cast<const TestClientSettings*>(curSettingsProfile.get());
	if (typedSettingsPtr == nullptr)
	{
		assert(typedSettingsPtr);
		return false;
	}

	m_appDataSrvIP = typedSettingsPtr->appDataService_appDataReceivingIP;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readAppSignalSet(const QByteArray& fileData)
{
	QElapsedTimer responseTime;
	responseTime.start();

	bool result = m_protoAppSignalSet.ParseFromArray(fileData.constData(), fileData.size());
	if (result == false)
	{
		qDebug() << __FUNCTION__ << "Error parsing AppSignalSet";
		return false;
	}

	qDebug() << "Parse AppSignalSet - Ok" << ", Time of parsing:" << responseTime.elapsed() << "ms";

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ConfigSocket::readAppDataSource(const QByteArray& fileData)
{
	if (m_pscore == nullptr)
	{
		assert(m_pscore);
		return false;
	}

	m_pscore->clearAllBases();

	QElapsedTimer responseTime;
	responseTime.start();

	//
	QVector<DataSource> dataSources;

	bool result = DataSourcesXML<DataSource>::readFromXml(fileData, &dataSources);
	if (result == false)
	{
		qDebug() << "Error reading AppDataSources from XML-file!";
		return false;
	}

	int dataSourcesCount = dataSources.count();
	for(int i = 0; i < dataSourcesCount; i++)
	{
		const DataSource& ds = dataSources[i];

		// Source Info
		//
		PS::SourceInfo si;

		si.index = i;

		si.caption = ds.lmCaption();
		si.equipmentID = ds.lmEquipmentID();

		si.moduleType = ds.lmModuleType();
		si.subSystem = ds.lmSubsystemID();
		si.frameCount = ds.lmRupFramesQuantity();
		si.dataID = ds.lmDataID();

		si.lmIP = ds.lmAddressPort();
		si.appDataSrvIP = m_appDataSrvIP;

		si.signalCount = ds.associatedSignals().count();

		// Source
		//
		PS::Source source;

		source.info() = si;
		source.associatedSignalList() = ds.associatedSignals();
		source.frameBase().setFrameCount(si.frameCount);

		m_pscore->sourceBase().append(source);
	}

	qDebug() << "Application Data Sources were loaded:" << dataSourcesCount << ", Time for load:" << responseTime.elapsed() << "ms";

	emit sourceBaseLoaded();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::loadSignalBase(ConfigSocket* pThis)
{
	if (pThis == nullptr)
	{
		assert(pThis);
		return;
	}

	if (pThis->m_pscore == nullptr)
	{
		assert(pThis->m_pscore);
		return;
	}

	QElapsedTimer responseTime;
	responseTime.start();

	int persentage = 0;

	pThis->m_pscore->signalBase().clear();	// clear signals

	int signalCount = pThis->m_protoAppSignalSet.appsignal_size();

	#ifndef Q_CONSOLE_APP
		for(int i = 0; i < signalCount; i++)
		{
			//
			//
			persentage = i * 100 / signalCount;

			if (persentage % 5 == 0)
			{
				emit pThis->signalBaseLoading(persentage);
			}

			//
			//
			const ::Proto::AppSignal& protoAppSignal = pThis->m_protoAppSignalSet.appsignal(i);

			PS::Signal signal;
			signal.serializeFrom(protoAppSignal);

			pThis->m_pscore->signalBase().append(signal);
		}
	#else
		qDebug() << "Wait, please, signals are loading ...";

		for(int i = 0; i < signalCount; i++)
		{
			const ::Proto::AppSignal& protoAppSignal = pThis->m_protoAppSignalSet.appsignal(i);

			PS::Signal signal;
			signal.serializeFrom(protoAppSignal);

			pThis->m_pscore->signalBase().append(signal);
		}
	#endif

	qDebug() << "Signals were loaded:" << signalCount << ", Time for load:" << responseTime.elapsed() << "ms";

	emit pThis->signalBaseLoaded();

	pThis->m_pscore->loadSignalsInSources();
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::startConnectionStateTimer()
{
	if (m_connectionStateTimer == nullptr)
	{
		m_connectionStateTimer = new QTimer(this);
		connect(m_connectionStateTimer, &QTimer::timeout, this, &ConfigSocket::updateConnectionState);
	}

	m_connectionStateTimer->start(CONFIG_SOCKET_TIMEOUT_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::stopConnectionStateTimer()
{
	if (m_connectionStateTimer != nullptr)
	{
		m_connectionStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ConfigSocket::updateConnectionState()
{
	if (m_cfgLoaderThread == nullptr)
	{
		return;
	}

	Tcp::ConnectionState&& state = m_cfgLoaderThread->getConnectionState();

	if (state.isConnected != m_connected)
	{
		m_connected = state.isConnected;

		if (m_connected == true)
		{
			m_cfgSrvIP = state.peerAddr;

			emit socketConnected();
		}
		else
		{
			m_cfgSrvIP.clear();

			emit socketDisconnected();
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
