#include "MonitorConfigController.h"
#include "Settings.h"
#include <QDomElement>
#include <QDomNodeList>
#include "version.h"

ConfigConnection::ConfigConnection(QString EquipmentId, QString ipAddress, int port) :
	m_equipmentId(EquipmentId),
	m_ip(ipAddress),
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
	HostAddressPort h(m_ip, m_port);
	return h;
}

MonitorConfigController::MonitorConfigController(HostAddressPort address1, HostAddressPort address2)
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	// Communication instance no
	//
	m_appInstanceSharedMemory.setKey("MonitorInstanceNo");
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
			QMessageBox::critical(nullptr,
								  qApp->applicationName(),
								  QString("Cannot create or attach to shared memory to determine software instance no. Error: %1")
								  .arg(m_appInstanceSharedMemory.errorString()));

			// Set "Some" Application Instance No
			//
			m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
		}
		else
		{
			// Get empty slot from shared memory
			//
			assert(m_appInstanceSharedMemory.isAttached() == true);

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
				assert(m_appInstanceNo > 0);

				QMessageBox::critical(nullptr,
									  qApp->applicationName(),
									  tr("Cannot determine software instance no. It seems all slots are occupied"));

				// Set "Some" Application Instance No
				//
				m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
			}

			m_appInstanceSharedMemory.unlock();
		}
	}

	qDebug() << "MonitorInstanceNo: " << m_appInstanceNo;

	// --
	//
	m_cfgLoaderThread = new CfgLoaderThread(theSettings.instanceStrId(), m_appInstanceNo, address1,  address2, false, nullptr, E::SoftwareType::Monitor, 0, 1, USED_SERVER_COMMIT_NUMBER);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &MonitorConfigController::slot_configurationReady);

	return;
}

MonitorConfigController::~MonitorConfigController()
{
	// Release application instance slot
	//
	if (m_appInstanceNo != -1)
	{
		assert(m_appInstanceSharedMemory.isAttached() == true);

		m_appInstanceSharedMemory.lock();

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
		sharedData[m_appInstanceNo] = 0;

		m_appInstanceSharedMemory.unlock();
	}

	// Stop communication
	//
	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
}

bool MonitorConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlocked(pathFileName, fileData, errorStr);

	if (result == false)
	{
		qDebug() << "MonitorConfigController::getFileBlocked: Can't get file " << pathFileName;
	}

	return result;
}

bool MonitorConfigController::getFile(const QString& pathFileName, QByteArray* fileData)
{
	Q_UNUSED(pathFileName);
	Q_UNUSED(fileData);

	// To do
	//
	assert(false);
	return false;
}

bool MonitorConfigController::getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlockedByID(id, fileData, errorStr);

	if (result == false)
	{
		qDebug() << "MonitorConfigController::getFileBlocked: Can't get file with ID" << id;
	}

	return result;
}

bool MonitorConfigController::getFileById(const QString& id, QByteArray* fileData)
{
	Q_UNUSED(id);
	Q_UNUSED(fileData);

	// To do
	//
	assert(false);
	return false;
}

Tcp::ConnectionState MonitorConfigController::getConnectionState() const
{
	Tcp::ConnectionState result;

	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);

		result.isConnected = false;
		return result;
	}

	result = m_cfgLoaderThread->getConnectionState();

	return result;
}

void MonitorConfigController::start()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);
		return;
	}

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	return;
}

void MonitorConfigController::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray /*buildFileInfoArray*/)
{
	qDebug() << "MonitorConfigThread::slot_configurationReady";

	ConfigSettings readSettings;

	// Get GlobalScript.js file
	//
	{
		QString parsingError;

		QByteArray ba;
		QString globalScriptFileName = "/" + theSettings.instanceStrId() + "/GlobalScript.js";
		bool ok = m_cfgLoaderThread->getFileBlocked(globalScriptFileName, &ba, &parsingError);
		if (ok == true)
		{
			readSettings.globalScript = QString(ba);
		}
		else
		{
			qDebug() << "ERROR: Cannot get GlobalScript.js, " << parsingError;
		}
	}

	// Parse XML
	//
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
			QDomNodeList settingsNodes = configElement.elementsByTagName("Settings");
			if (settingsNodes.size() != 1)
			{
				readSettings.errorMessage += tr("Parsing Settings node error.\n");
			}
			else
			{
				result &= xmlReadSettingsNode(settingsNodes.item(0), &readSettings);
			}
		}

		// Error handling
		//
		if (result == false ||
			readSettings.errorMessage.isEmpty() == false)
		{
			QString completeErrorMessage = tr("Parsing configuration file error: %1").arg(readSettings.errorMessage);

			qDebug() << completeErrorMessage;
			QMessageBox::critical(nullptr, qApp->applicationName(), completeErrorMessage);
		}
	}

	// Get all schema details
	//
	{
		// Get SchemaDetails.pbuf file
		//
		QString parsingError;

		QByteArray ba;
		QString fileName = "/" + theSettings.instanceStrId() + "/SchemaDetails.pbuf";
		bool ok = m_cfgLoaderThread->getFileBlocked(fileName, &ba, &parsingError);

		if (ok == false)
		{
			qDebug() << "ERROR: Cannot get " << fileName << ", " << parsingError;

			QMutexLocker locker(&m_mutex);
			m_schemaDetailsSet.clear();
		}
		else
		{
			QMutexLocker locker(&m_mutex);
			m_schemaDetailsSet.clear();

			m_schemaDetailsSet.Load(ba);
		}
	}

	// Trace received params
	//
	qDebug() << "New configuration arrived";
	qDebug() << "StartSchemaID: " << readSettings.startSchemaId;
	qDebug() << "ADS1 (id, ip, port): " << readSettings.appDataService1.equipmentId() << ", " << readSettings.appDataService1.ip() << ", " << readSettings.appDataService1.port();
	qDebug() << "ADS2 (id, ip, port): " << readSettings.appDataService2.equipmentId() << ", " << readSettings.appDataService2.ip() << ", " << readSettings.appDataService2.port();


	// Emit signal to inform everybody about new configuration
	//
	{
		QMutexLocker locker(&m_confugurationMutex);
		m_configuration = readSettings;
	}

	emit configurationArrived(readSettings);

	return;
}

bool MonitorConfigController::xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		buildInfoNode.nodeName() != "BuildInfo")
	{
		assert(outSetting);
		assert(buildInfoNode.nodeName() == "BuildInfo");
		return false;
	}

	QDomElement element = buildInfoNode.toElement();

	outSetting->project = element.attribute(QLatin1String("Project"));

	return true;
}

bool MonitorConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		softwareNode.nodeName() != "Software")
	{
		assert(outSetting);
		assert(softwareNode.nodeName() == "Software");
		return false;
	}

	QDomElement softwareElement = softwareNode.toElement();

	// Read StrID attribute
	//
	QString appEquipmentId = softwareElement.attribute("ID");

	if (theSettings.instanceStrId() != appEquipmentId)
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

	if (softwareType != E::SoftwareType::Monitor)
	{
		// The received file has different type then expected,
		//
		outSetting->errorMessage += "The received file has different software type then expected.\n";
		return false;
	}

	return outSetting->errorMessage.isEmpty();
}

bool MonitorConfigController::xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		settingsNode.nodeName() != "Settings")
	{
		assert(outSetting);
		assert(settingsNode.nodeName() == "Settings");
		return false;
	}

	QDomElement settingsElement = settingsNode.toElement();

	// Check if XML contains Error tag
	//
	QDomNodeList errorNodes = settingsElement.elementsByTagName("Error");

	if (errorNodes.isEmpty() == false)
	{
		for (int i = 0; i < errorNodes.count();  i++)
		{
			outSetting->errorMessage += QString("%1\n").arg(errorNodes.at(i).toElement().text());
		}
		return false;
	}

	// Get StartSchemaID data
	//
	{
		QDomNodeList startSchemaNodes = settingsElement.elementsByTagName("StartSchemaID");

		if (startSchemaNodes.isEmpty() == true)
		{
			outSetting->errorMessage += tr("Cannot find StartSchemaID tag %1\n");
			return false;
		}
		else
		{
			outSetting->startSchemaId = startSchemaNodes.at(0).toElement().text();
		}
	}

	// Get AppDataService data
	//
	{
		QDomNodeList dasNodes = settingsElement.elementsByTagName("AppDataService");

		if (dasNodes.isEmpty() == true)
		{
			outSetting->errorMessage += tr("Cannot find AppDataService tag %1\n");
			return false;
		}
		else
		{
			QDomElement dasXmlElement = dasNodes.at(0).toElement();

			QString dasId1 = dasXmlElement.attribute("AppDataServiceID1");
			QString dasIp1 = dasXmlElement.attribute("ip1");
			int dasPort1 = dasXmlElement.attribute("port1").toInt();

			QString dasId2 = dasXmlElement.attribute("AppDataServiceID2");
			QString dasIp2 = dasXmlElement.attribute("ip2");
			int dasPort2 = dasXmlElement.attribute("port2").toInt();

			outSetting->appDataService1 = ConfigConnection(dasId1, dasIp1, dasPort1);
			outSetting->appDataService2 = ConfigConnection(dasId2, dasIp2, dasPort2);
		}
	}

	// Get ArchiveService data
	//
	{
		QDomNodeList dasNodes = settingsElement.elementsByTagName("ArchiveService");

		if (dasNodes.isEmpty() == true)
		{
			outSetting->errorMessage += tr("Cannot find ArchiveService tag %1\n");
			return false;
		}
		else
		{
			QDomElement dasXmlElement = dasNodes.at(0).toElement();

			QString asId1 = dasXmlElement.attribute("AppDataServiceID1");
			QString asIp1 = dasXmlElement.attribute("ip1");
			int asPort1 = dasXmlElement.attribute("port1").toInt();

			QString asId2 = dasXmlElement.attribute("AppDataServiceID2");
			QString asIp2 = dasXmlElement.attribute("ip2");
			int asPort2 = dasXmlElement.attribute("port2").toInt();

			outSetting->archiveService1 = ConfigConnection(asId1, asIp1, asPort1);
			outSetting->archiveService2 = ConfigConnection(asId2, asIp2, asPort2);
		}
	}

	return outSetting->errorMessage.isEmpty();
}


std::vector<VFrame30::SchemaDetails> MonitorConfigController::schemasDetails() const
{
	QMutexLocker l(&m_mutex);

	std::vector<VFrame30::SchemaDetails> result = m_schemaDetailsSet.schemasDetails();

	return result;
}

std::set<QString> MonitorConfigController::schemaAppSignals(const QString& schemaId)
{
	QMutexLocker l(&m_mutex);

	std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetailsSet.schemaDetails(schemaId);

	if (details == nullptr)
	{
		return std::set<QString>();
	}

	return details->m_signals;
}


ConfigSettings MonitorConfigController::configuration() const
{
	QMutexLocker locker(&m_confugurationMutex);
	return m_configuration;
}
