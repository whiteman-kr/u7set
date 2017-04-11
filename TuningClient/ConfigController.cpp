#include "Stable.h"
#include "ConfigController.h"
#include "../lib/Tuning/TuningFilter.h"
#include "TuningObjectManager.h"
#include "MainWindow.h"
#include "../lib/ServiceSettings.h"

//
// ConfigController
//

ConfigController::ConfigController(QWidget *parent, HostAddressPort address1, HostAddressPort address2)
	:m_parent(parent),
	  m_address1(address1),
	  m_address2(address2)
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	// Communication instance no
	//
	m_appInstanceSharedMemory.setKey("TuningClientInstanceNo");
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
                                  tr("Cannot create or attach to shared memory to determine software instance no. Error: %1")
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

	//qDebug() << "TuningClientInstanceNo: " << m_appInstanceNo;

	// --
	//
	m_cfgLoaderThread = new CfgLoaderThread(theSettings.instanceStrId(), m_appInstanceNo, address1,  address2);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &ConfigController::slot_configurationReady);

	return;
}

ConfigController::~ConfigController()
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

bool ConfigController::requestObjectFilters()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_TUNING_FILTERS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get ObjectFilters.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		addEventMessage(completeErrorMessage);
		return false;
	}
	else
	{
		if (theFilters.load(data, &errorStr, true) == false)
		{
			QString completeErrorMessage = tr("ObjectFilters.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			addEventMessage(completeErrorMessage);
			return false;
		}
		else
		{
			QString message = tr("Received file: ObjectFilters.xml");
			addEventMessage(message);
		}
	}

	return true;
}

bool ConfigController::requestSchemasDetails()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_TUNING_SCHEMAS_DETAILS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get SchemasDetails.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		addEventMessage(completeErrorMessage);
		return false;
	}
	else
	{
		if (theFilters.loadSchemasDetails(data, &errorStr) == false)
		{
			QString completeErrorMessage = tr("SchemasDetails.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			addEventMessage(completeErrorMessage);
			return false;
		}
		else
		{
			QString message = tr("Received file: SchemasDetails.xml");
			addEventMessage(message);
		}
	}

	return true;
}

bool ConfigController::requestTuningSignals()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_TUNING_SIGNALS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get TuningSignals.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		addEventMessage(completeErrorMessage);
		return false;
	}
	else
	{
		if (theObjectManager->loadDatabase(data, &errorStr) == false)
		{
			QString completeErrorMessage = tr("TuningSignals.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			addEventMessage(completeErrorMessage);
			return false;
		}
		else
		{
			QString message = tr("Received file: TuningSignals.xml");
			addEventMessage(message);
		}
	}

	return true;
}

/*bool ConfigController::requestSchema(const QString &schemaID)
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(schemaID, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get %1 file error: %2").arg(schemaID).arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		addEventMessage(completeErrorMessage);
		return false;
	}
	else
	{
		QString message = tr("Received schema: %1").arg(schemaID);
		addEventMessage(message);
	}
	return true;
}*/

Tcp::ConnectionState ConfigController::getConnectionState() const
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

QString ConfigController::getStateToolTip()
{
	Tcp::ConnectionState connectionState = getConnectionState();

	QString result = tr("Configuration Service connection\r\n\r\n");
	result += tr("IP address (primary): %1\r\n").arg(m_address1.addressPortStr());
	result += tr("IP address (secondary): %1\r\n").arg(m_address2.addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established\r\n") : tr("no connection\r\n"));

	if (m_eventLog.empty() == false)
	{
		result += tr("\r\n");
		for (auto s : m_eventLog)
		{
			result += s + "\r\n";
		}
	}

	return result;
}

void ConfigController::start()
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

void ConfigController::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{
	ConfigSettings readSettings;

	QString message = tr("New configuration arrived");
	addEventMessage(message);

	// Parse XML
	//
	QDomDocument xml;
	QString parsingError;

	bool result = xml.setContent(configurationXmlData, false, &parsingError);

	if (result == false)
	{
		readSettings.errorMessage += parsingError + "\n";
	}
	else
	{
		// Get <Configuration>
		//
		QDomElement configElement = xml.documentElement();

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

		// Schemas node
		//
		QDomNodeList schemasNodes = configElement.elementsByTagName("Schemas");
		if (schemasNodes.size()  > 0)
		{
			result &= xmlReadSchemasNode(schemasNodes.item(0), buildFileInfoArray, &readSettings);
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
		addEventMessage(completeErrorMessage);

	}

	// Trace received params
	//
	theLogFile->writeMessage(tr("New configuration arrived"));
	theLogFile->writeMessage(tr("TUNS1 (id, ip, port): %1, %2, %3").arg(readSettings.tuns1.equipmentId()).arg(readSettings.tuns1.ip()).arg(readSettings.tuns1.port()));
	theLogFile->writeMessage(tr("TUNS2 (id, ip, port): %1, %2, %3").arg(readSettings.tuns2.equipmentId()).arg(readSettings.tuns2.ip()).arg(readSettings.tuns2.port()));

	bool someFilesUpdated = false;


	for (const Builder::BuildFileInfo& f: buildFileInfoArray)
	{

		// Add MD5 record to a map, if it does not exist
		//

		if (m_filesMD5Map.find(f.ID) == m_filesMD5Map.end())
		{
			m_filesMD5Map[f.ID] = "";
		}

		// Get previous MD5
		//

		const QString& md5 = m_filesMD5Map[f.ID];

		if (md5 != f.md5)
		{
			m_filesMD5Map[f.ID] = f.md5;

			someFilesUpdated = true;

			// Filters

			if (f.ID == CFG_FILE_ID_TUNING_FILTERS)
			{
				requestObjectFilters();
			}

			// Details

			if (f.ID == CFG_FILE_ID_TUNING_SCHEMAS_DETAILS)
			{
				requestSchemasDetails();
			}

			// Signals

			if (f.ID == CFG_FILE_ID_TUNING_SIGNALS)
			{
				requestTuningSignals();
			}

			/*
			// Schemas

			for (auto schemaID : readSettings.schemaFiles)
			{
				if (f.ID == schemaID)
				{
					requestSchema(f.ID);
				}
			}
			*/
		}
	}

	if (someFilesUpdated == true)
	{
		// Emit signal to inform everybody about new configuration
		//

		emit configurationArrived(readSettings);
	}

	return;
}

bool ConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlocked(pathFileName, fileData, errorStr);

	if (result == false)
	{
		QString message = tr("ConfigController::getFileBlocked: Can't get file ") + pathFileName;
		theLogFile->writeError(message);

		message = tr("Can't receive file: ") + pathFileName;
		addEventMessage(message);
	}
	else
	{
		QString message = tr("Received file: ") + pathFileName;
		addEventMessage(message);
	}

	return result;
}

bool ConfigController::getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr)
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread != nullptr);
		return false;
	}

	bool result = m_cfgLoaderThread->getFileBlockedByID(id, fileData, errorStr);

	if (result == false)
	{
		theLogFile->writeError(tr("ConfigController::getFileBlocked: Can't get file with ID ") + id);

		QString message = tr("Can't receive file by ID: ") + id;
		addEventMessage(message);
	}

	return result;
}

bool ConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting)
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
        outSetting->errorMessage += tr("The received file has different EquipmentID then expected.\n");
		return false;
	}

	// Read Type attribute
	//
	int softwareType = softwareElement.attribute("Type").toInt();

	if (softwareType != E::SoftwareType::TuningClient)
	{
		// The received file has different type then expected,
		//
        outSetting->errorMessage += tr("The received file has different software type then expected.\n");
		return false;
	}

	return outSetting->errorMessage.isEmpty();
}

bool ConfigController::xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting)
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

	// Get TuningService data
	//
	{
		QDomNodeList dasNodes = settingsElement.elementsByTagName("TuningService");

		if (dasNodes.isEmpty() == true)
		{
			outSetting->errorMessage += tr("Cannot find TuningService tag %1\n");
			return false;
		}
		else
		{
			QDomElement dasXmlElement = dasNodes.at(0).toElement();

			QString tunsId1 = dasXmlElement.attribute("TuningServiceID1");
			QString tunsIp1 = dasXmlElement.attribute("ip1");
			int tunsPort1 = dasXmlElement.attribute("port1").toInt();

			QString tunsId2 = dasXmlElement.attribute("TuningServiceID2");
			QString tunsIp2 = dasXmlElement.attribute("ip2");
			int tunsPort2 = dasXmlElement.attribute("port2").toInt();

			outSetting->autoApply = dasXmlElement.attribute("autoApply") == "true" ? true : false;

			outSetting->tuns1 = ConfigConnection(tunsId1, tunsIp1, tunsPort1);
			outSetting->tuns2= ConfigConnection(tunsId2, tunsIp2, tunsPort2);

		}
	}

	return outSetting->errorMessage.isEmpty();
}

bool ConfigController::xmlReadSchemasNode(const QDomNode& schemasNode, const BuildFileInfoArray& buildFileInfoArray, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		schemasNode.nodeName() != "Schemas")
	{
		assert(outSetting);
		assert(schemasNode.nodeName() == "Schemas");
		return false;
	}

	QDomElement schemasElement = schemasNode.toElement();

	// Check if XML contains Error tag
	//
	QDomNodeList errorNodes = schemasElement.elementsByTagName("Error");

	if (errorNodes.isEmpty() == false)
	{
		for (int i = 0; i < errorNodes.count();  i++)
		{
			outSetting->errorMessage += QString("%1\n").arg(errorNodes.at(i).toElement().text());
		}
		return false;
	}

	// Get Schemas
	//
	{
		QDomNodeList dasNodes = schemasElement.elementsByTagName("Schema");

		for (int i = 0; i <dasNodes.size(); i++)
		{
			QDomElement dasXmlElement = dasNodes.at(i).toElement();

			QString schemaId = dasXmlElement.text();

			for (const Builder::BuildFileInfo& f: buildFileInfoArray)
			{
				if (f.ID == schemaId)
				{
					outSetting->schemasID.push_back(f.ID);
				}
			}
		}
	}

	QString message = tr("Schemas count: %1").arg(outSetting->schemasID.size());
	addEventMessage(message);

	return outSetting->errorMessage.isEmpty();
}

void ConfigController::addEventMessage(const QString& text)
{
	m_eventLog.push_back(text);

	while (m_eventLog.size() > 10)
	{
		m_eventLog.removeFirst();
	}
}
