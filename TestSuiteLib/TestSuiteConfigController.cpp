#include "../lib/SoftwareSettings.h"
#include "TestSuiteConfigController.h"

// ------------------------- ConfigConnection -----------------------------------

ConfigConnection::ConfigConnection(QString EquipmentId, QString ipAddress, int port) :
	m_equipmentId(std::move(EquipmentId)),
	m_ip(std::move(ipAddress)),
	m_port(port)
{
}

QString ConfigConnection::equipmentId() const
{
	return m_equipmentId;
}

QString ConfigConnection::ip() const
{
	return m_ip;
}

int ConfigConnection::port() const
{
	return m_port;
}

HostAddressPort ConfigConnection::address() const
{
	return HostAddressPort{m_ip, static_cast<quint16>(m_port)};
}

// ------------------------- TestSuiteConfigController -----------------------------------

TestSuiteConfigController::TestSuiteConfigController(const QString& instanceId,
													 HostAddressPort address1,
													 HostAddressPort address2,
													 ILogFile* appLogFile) :
	HasLogFile(appLogFile, "ConfigController")
{
	m_softwareInfo.init(E::SoftwareType::TestSuite, instanceId, 0, 1);

	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	// Communication instance no
	//
	m_appInstanceSharedMemory.setKey("TestSuiteInstanceNo");
	int maxInstanceCount = 512;

	bool ok = m_appInstanceSharedMemory.create(maxInstanceCount * sizeof(qint64));

	if (ok == true)
	{
		// Shared memory created, initialize it
		//
		m_appInstanceSharedMemory.lock();

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());

		for (int i = 0; i < maxInstanceCount; i++)
		{
			sharedData[i] = 0;
		}

		sharedData[0] = qApp->applicationPid();
		m_appInstanceNo = 0;

		m_appInstanceSharedMemory.unlock();
	}
	else
	{
		if (m_appInstanceSharedMemory.error() == QSharedMemory::SharedMemoryError::AlreadyExists)
		{
			ok = m_appInstanceSharedMemory.attach();
		}

		if (ok == false)
		{
			qDebug() << QString("Cannot create or attach to shared memory to determine software instance no. Error: %1")
						.arg(m_appInstanceSharedMemory.errorString());

			// Set "Some" Application Instance No
			//
			m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
		}
		else
		{
			// Get empty slot from shared memory
			//
			Q_ASSERT(m_appInstanceSharedMemory.isAttached() == true);

			m_appInstanceSharedMemory.lock();

			qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
			m_appInstanceNo = -1;

			for (int i = 0; i < maxInstanceCount; i++)
			{
				if (sharedData[i] == 0)
				{
					// This is an empty slot, use it
					//
					sharedData[i] = qApp->applicationPid();	// 1 means
					m_appInstanceNo = i;

					break;
				}
			}

			if (m_appInstanceNo == -1)
			{
				Q_ASSERT(m_appInstanceNo > 0);

				qDebug() << tr("Cannot determine software instance no. It seems all slots are occupied");

				// Set "Some" Application Instance No
				//
				m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
			}

			m_appInstanceSharedMemory.unlock();
		}
	}

	// --
	//
	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											m_appInstanceNo,
											address1,
											address2,
											false,
											std::make_shared<CircularLogger>(appLogFile, "CfgLoaderThread"));

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_unknownClientID, this, [this]()
	{
		writeError(tr("Unknown client %1").arg(m_softwareInfo.equipmentID()));
	});

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_wrongClientHostname, this, [this]()
	{
		writeError(tr("Running on computer with wrong hostanme %1").arg(m_softwareInfo.equipmentID()));
	});

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &TestSuiteConfigController::onConfigurationReady);

	return;
}

TestSuiteConfigController::~TestSuiteConfigController()
{
	// Release application instance slot
	//
	if (m_appInstanceNo != -1)
	{
		Q_ASSERT(m_appInstanceSharedMemory.isAttached() == true);

		m_appInstanceSharedMemory.lock();

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
		sharedData[m_appInstanceNo] = 0;

		m_appInstanceSharedMemory.unlock();
	}

	// Stop communication
	//
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread);
	}
	else
	{
		m_cfgLoaderThread->quit();
		delete m_cfgLoaderThread;
	}
}

void TestSuiteConfigController::setConnectionParams(QString equipmentId, HostAddressPort address1, HostAddressPort address2)
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread);
		return;
	}

	m_softwareInfo.setEquipmentID(equipmentId);

	m_cfgLoaderThread->setConnectionParams(m_softwareInfo, address1, address2, true);

	return;
}

bool TestSuiteConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlocked(pathFileName, fileData, errorStr);
	if (result == false)
	{
		writeError(tr("getFileBlocked() Can't get file %1").arg(pathFileName));
	}
	else
	{
		writeMessage(tr("getFileBlocked('%1') Success").arg(pathFileName));
	}

	return result;
}

bool TestSuiteConfigController::getFile(const QString& pathFileName, QByteArray* fileData)
{
	Q_UNUSED(pathFileName);
	Q_UNUSED(fileData);

	// To do
	//
	Q_ASSERT(false);
	return false;
}

bool TestSuiteConfigController::getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlockedByID(id, fileData, errorStr);

	if (result == false)
	{
		writeError(tr("getFileBlockedById() Can't get fileid %1").arg(id));
	}

	return result;
}

bool TestSuiteConfigController::getFileById(const QString& id, QByteArray* fileData)
{
	Q_UNUSED(id);
	Q_UNUSED(fileData);

	// To do
	//
	Q_ASSERT(false);
	return false;
}

bool TestSuiteConfigController::hasFileId(QString fileId) const
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread != nullptr);
		return false;
	}

	return m_cfgLoaderThread->hasFileID(std::move(fileId));
}

Tcp::ConnectionState TestSuiteConfigController::getConnectionState() const
{
	Tcp::ConnectionState result;

	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread);

		result.isConnected = false;
		return result;
	}

	result = m_cfgLoaderThread->getConnectionState();

	return result;
}

const SoftwareInfo& TestSuiteConfigController::softwareInfo() const
{
	return m_softwareInfo;
}

void TestSuiteConfigController::start()
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread);
		return;
	}

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	return;
}

void TestSuiteConfigController::onConfigurationReady(const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	Q_UNUSED(buildFileInfoArray);
	Q_UNUSED(sessionParams);


	writeMessage(tr("New configuration arrived"));

	// Load settings

	ConfigSettings readSettings;

	{
		QString parsingError;
		QDomDocument xml;

		int errorLine = 0;
		int errorColumn = 0;

		bool result = xml.setContent(configurationXmlData, false, &parsingError, &errorLine, &errorColumn);

		if (result == false)
		{
			QString errorStr = tr("%1, line %2, column %3").arg(parsingError).arg(errorLine).arg(errorColumn);
			readSettings.errorMessage += errorStr + "\n";
		}
		else
		{
			// Get <Configuration>
			//
			QDomElement configElement = xml.documentElement();

			// BuildInfo node
			//
			QDomNodeList buildInfoNodes = configElement.elementsByTagName("BuildInfo");
			if (buildInfoNodes.size() != 1)
			{
				readSettings.errorMessage += tr("Parsing BuildInfo node error.\n");
			}
			else
			{
				result &= xmlReadBuildInfoNode(buildInfoNodes.item(0), &readSettings);
			}

			// Software node
			//
			QDomNodeList softwareNodes = configElement.elementsByTagName("Software");
			if (softwareNodes.size() != 1)
			{
				readSettings.errorMessage += tr("Parsing Software node error.\n");
			}
			else
			{
				result &= xmlReadSoftwareNode(softwareNodes.item(0), &readSettings);
			}

			// Settings node
			//
			result &= applyCurSettingsProfile(curSettingsProfile, &readSettings);
		}

		// Error handling
		//
		if (result == false ||
			readSettings.errorMessage.isEmpty() == false)
		{
			writeError(tr("Parsing configuration file error: %1").arg(readSettings.errorMessage));
		}
	}

	// Get test files list

	for (const Builder::BuildFileInfo& buildFileInfo: buildFileInfoArray)
	{
		if (buildFileInfo.pathFileName.endsWith(".js") == false)
		{
			continue;
		}

		readSettings.scriptFiles.push_back(buildFileInfo.pathFileName);
	}

	// Trace received params
	//
	writeMessage(tr("ADS1 (id, ip, port): %1, %2, %3").arg(readSettings.appDataService1.equipmentId()).arg(readSettings.appDataService1.ip()).arg(readSettings.appDataService1.port()));
	writeMessage(tr("ADS2 (id, ip, port): %1, %2, %3").arg(readSettings.appDataService2.equipmentId()).arg(readSettings.appDataService2.ip()).arg(readSettings.appDataService2.port()));

	writeMessage(tr("ADS RT Trends 1 (id, ip, port): %1, %2, %3").arg(readSettings.appDataServiceRealtimeTrend1.equipmentId()).arg(readSettings.appDataServiceRealtimeTrend1.ip()).arg(readSettings.appDataServiceRealtimeTrend1.port()));
	writeMessage(tr("ADS RT Trends 2 (id, ip, port): %1, %2, %3").arg(readSettings.appDataServiceRealtimeTrend2.equipmentId()).arg(readSettings.appDataServiceRealtimeTrend2.ip()).arg(readSettings.appDataServiceRealtimeTrend2.port()));

	writeMessage(tr("ArchiveService1 (id, ip, port): %1, %2, %3").arg(readSettings.archiveService1.equipmentId()).arg(readSettings.archiveService1.ip()).arg(readSettings.archiveService1.port()));
	writeMessage(tr("ArchiveService2 (id, ip, port): %1, %2, %3").arg(readSettings.archiveService2.equipmentId()).arg(readSettings.archiveService2.ip()).arg(readSettings.archiveService2.port()));

	writeMessage(QString("TuningEnabled = %1").arg(readSettings.tuningEnabled));
	if (readSettings.tuningEnabled == true)
	{
		for (const TestSuiteSettings::TuningService& ts : readSettings.tuningServices)
		{
			writeMessage(tr("TuningService (id, ip, port): %1, %2, %3").arg(ts.tuningServiceID).arg(ts.clientRequestIP).arg(ts.clientRequestPort));
			writeMessage(tr("TuningSources: %1").arg(ts.drivenSources.join(", ")));
		}
	}

	if (readSettings.errorMessage.isEmpty() == false)
	{
		writeError(tr("%1").arg(readSettings.errorMessage));
	}

	// --
	//
	{
		QWriteLocker locker(&m_confugurationLock);
		readSettings.id = m_configuration.id + 1;
		m_configuration = std::move(readSettings);		// Cannot move readSettings here as it is used later for `emit configurationArrived(readSettings)`
	}

	// Emit signal to inform everybody about new configuration
	//
	emit configurationArrived();

	return;
}

bool TestSuiteConfigController::xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr)
	{
		Q_ASSERT(outSetting);
		return false;
	}

	if (buildInfoNode.nodeName() != "BuildInfo")
	{
		Q_ASSERT(buildInfoNode.nodeName() == "BuildInfo");
		return false;
	}

	QDomElement element = buildInfoNode.toElement();

	outSetting->buildNo = element.attribute(QLatin1String("ID")).toInt();
	outSetting->project = element.attribute(QLatin1String("Project"));

	return true;
}

bool TestSuiteConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr)
	{
		Q_ASSERT(outSetting);
		return false;
	}

	if (softwareNode.nodeName() != "Software")
	{
		Q_ASSERT(softwareNode.nodeName() == "Software");
		return false;
	}

	QDomElement softwareElement = softwareNode.toElement();

	// Read StrID attribute
	//
	QString appEquipmentId = softwareElement.attribute(EquipmentPropNames::EQUIPMENT_ID);

	if (m_softwareInfo.equipmentID() != appEquipmentId)
	{
		// The received file has different StrID then expected
		//
		outSetting->errorMessage += "The received file has different EquipmentID then expected.\n";
		return false;
	}

	outSetting->softwareEquipmentId = appEquipmentId;

	// Read Type attribute
	//
	int softwareType = softwareElement.attribute("Type").toInt();

	if (softwareType != E::SoftwareType::TestSuite)
	{
		// The received file has different type then expected,
		//
		outSetting->errorMessage += "The received file has different software type then expected.\n";
		return false;
	}

	return outSetting->errorMessage.isEmpty();
}

bool TestSuiteConfigController::applyCurSettingsProfile(std::shared_ptr<const SoftwareSettings> curSettingsProfile, ConfigSettings* outSetting)
{
	if (curSettingsProfile == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (outSetting == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	const TestSuiteSettings* typedSettingsPtr = dynamic_cast<const TestSuiteSettings*>(curSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	const TestSuiteSettings& ms = *typedSettingsPtr;

	//

	outSetting->appDataService1 = ConfigConnection(ms.appDataServiceID1, ms.appDataServiceIP1, ms.appDataServicePort1);
	outSetting->appDataService2 = ConfigConnection(ms.appDataServiceID2, ms.appDataServiceIP2, ms.appDataServicePort2);

	outSetting->appDataServiceRealtimeTrend1 = ConfigConnection(ms.appDataServiceID1, ms.realtimeDataIP1, ms.realtimeDataPort1);
	outSetting->appDataServiceRealtimeTrend2 = ConfigConnection(ms.appDataServiceID2, ms.realtimeDataIP2, ms.realtimeDataPort2);

	//

	outSetting->archiveService1 = ConfigConnection(ms.archiveServiceID1, ms.archiveServiceIP1, ms.archiveServicePort1);
	outSetting->archiveService2 = ConfigConnection(ms.archiveServiceID2, ms.archiveServiceIP2, ms.archiveServicePort2);

	//

	outSetting->tuningEnabled = ms.tuningEnabled;

	if (ms.tuningEnabled == true)
	{
		outSetting->tuningServices = ms.tuningServices;
	}
	else
	{
		// tuning disabled
		//
		outSetting->tuningServices.clear();
	}

	return true;
}

ConfigSettings TestSuiteConfigController::configuration() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration;
}
