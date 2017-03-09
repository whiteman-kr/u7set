#include "Stable.h"
#include "Settings.h"
#include "ConfigController.h"
#include "../lib/TuningFilter.h"
#include "TuningObjectManager.h"
#include "MainWindow.h"
#include "../lib/ServiceSettings.h"


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

//
// ConfigController
//

ConfigController::ConfigController(QWidget *parent, HostAddressPort address1, HostAddressPort address2)
	:m_parent(parent)
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

	qDebug() << "TuningClientInstanceNo: " << m_appInstanceNo;

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
		theLogFile->writeError(tr("ConfigController::getFileBlocked: Can't get file ") + pathFileName);
	}

	return result;
}

bool ConfigController::getFile(const QString& pathFileName, QByteArray* fileData)
{
	Q_UNUSED(pathFileName);
	Q_UNUSED(fileData);

	// To do
	//
	assert(false);
	return false;
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
	}

	return result;
}

bool ConfigController::getFileById(const QString& id, QByteArray* fileData)
{
	Q_UNUSED(id);
	Q_UNUSED(fileData);

	// To do
	//
	assert(false);
	return false;
}

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

bool ConfigController::getObjectFilters()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_TUNING_FILTERS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get ObjectFilters.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		return false;
	}
	else
	{
        if (theFilters.load(data, &errorStr, true) == false)
		{
			QString completeErrorMessage = tr("ObjectFilters.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			return false;
		}
	}

	return true;
}

bool ConfigController::getSchemasDetails()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_SCHEMAS_DETAILS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get SchemasDetails.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		return false;
	}
	else
	{
		if (theFilters.loadSchemasDetails(data, &errorStr) == false)
		{
			QString completeErrorMessage = tr("SchemasDetails.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			return false;
		}
	}

	return true;}

bool ConfigController::getTuningSignals()
{
	QByteArray data;
	QString errorStr;
	if (getFileBlockedById(CFG_FILE_ID_TUNING_SIGNALS, &data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("getFileBlockedById: Get TuningSignals.xml file error: %1").arg(errorStr);
		theLogFile->writeError(completeErrorMessage);
		QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
		return false;
	}
	else
	{
        if (theObjectManager->loadDatabase(data, &errorStr) == false)
		{
			QString completeErrorMessage = tr("TuningSignals.xml file loading error: %1").arg(errorStr);
			theLogFile->writeError(completeErrorMessage);
			QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
			return false;
		}
	}

	return true;
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

	// Trace received params
	//
	theLogFile->writeMessage(tr("New configuration arrived"));
	theLogFile->writeMessage(tr("TUNS1 (id, ip, port): %1, %2, %3").arg(readSettings.tuns1.equipmentId()).arg(readSettings.tuns1.ip()).arg(readSettings.tuns1.port()));
	theLogFile->writeMessage(tr("TUNS2 (id, ip, port): %1, %2, %3").arg(readSettings.tuns2.equipmentId()).arg(readSettings.tuns2.ip()).arg(readSettings.tuns2.port()));

	readSettings.updateFilters = false;
	readSettings.updateSchemas = false;
	readSettings.updateSignals = false;

	QMutexLocker locker(&m_mutex);

	for (const Builder::BuildFileInfo& f: buildFileInfoArray)
	{
		if (f.ID == CFG_FILE_ID_TUNING_FILTERS)
		{
			if (m_md5Filters != f.md5)
			{
				readSettings.updateFilters = true;
				m_md5Filters = f.md5;
			}
		}
		if (f.ID == CFG_FILE_ID_SCHEMAS_DETAILS)
		{
			if (m_md5Schemas != f.md5)
			{
				readSettings.updateSchemas = true;
				m_md5Schemas = f.md5;
			}
		}
		if (f.ID == CFG_FILE_ID_TUNING_SIGNALS)
		{
			if (m_md5Signals != f.md5)
			{
				readSettings.updateSignals = true;
				m_md5Signals = f.md5;
			}
		}
	}

	// Emit signal to inform everybody about new configuration
	//
	emit configurationArrived(readSettings);

	return;
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

			outSetting->tuns1 = ConfigConnection(tunsId1, tunsIp1, tunsPort1);
			outSetting->tuns2= ConfigConnection(tunsId2, tunsIp2, tunsPort2);
		}
	}

	return outSetting->errorMessage.isEmpty();
}
