#include "ConfigController.h"
#include "MainWindow.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/ClientBehavior.h"

//
// ConfigController
//

ConfigController::ConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, QWidget* parent) :
	m_parent(parent),
	m_address1(address1),
	m_address2(address2)
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");
	qRegisterMetaType<HostAddressPort>("HostAddressPort");

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
			QMessageBox::critical(m_parent,
								  qApp->applicationName(),
								  tr("Cannot create or attach to shared memory to determine software instance no.\nError: %1")
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

				QMessageBox::critical(m_parent,
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
	m_cfgLoaderThread = new CfgLoaderThread(softwareInfo, m_appInstanceNo, address1,  address2, false, nullptr);

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

Tcp::ConnectionState ConfigController::getConnectionState() const
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);
		return Tcp::ConnectionState();
	}

	return m_cfgLoaderThread->getConnectionState();
}

QString ConfigController::getStateToolTip()
{
	if (m_cfgLoaderThread == nullptr)
	{
		assert(m_cfgLoaderThread);
		return QString();
	}

	Tcp::ConnectionState connectionState = m_cfgLoaderThread->getConnectionState();
	HostAddressPort currentConnection = m_cfgLoaderThread->getCurrentServerAddressPort();

	QString result = tr("Configuration Service connection\n\n");
	result += tr("Address (primary): %1\n").arg(m_address1.addressPortStr());
	result += tr("Address (secondary): %1\n\n").arg(m_address2.addressPortStr());
	result += tr("Address (current): %1\n").arg(currentConnection.addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established") : tr("no connection"));

	return result;
}

int ConfigController::schemaCount() const
{
	QMutexLocker l(&m_mutex);
	return m_schemaDetailsSet.schemaCount();
}

QString ConfigController::schemaCaptionById(const QString& schemaId) const
{
	QMutexLocker l(&m_mutex);
	return m_schemaDetailsSet.schemaCaptionById(schemaId);
}

QString ConfigController::schemaCaptionByIndex(int schemaIndex) const
{
	QMutexLocker l(&m_mutex);
	return m_schemaDetailsSet.schemaCaptionByIndex(schemaIndex);
}

QString ConfigController::schemaIdByIndex(int schemaIndex) const
{
	QMutexLocker l(&m_mutex);
	return m_schemaDetailsSet.schemaIdByIndex(schemaIndex);
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

void ConfigController::slot_configurationReady(const QByteArray configurationXmlData,
											   const BuildFileInfoArray buildFileInfoArray,
											   SessionParams sessionParams,
											   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	Q_UNUSED(sessionParams);

	// Copy old settings to new settings, EXCEPT schemas information!
	//
	ConfigSettings readSettings = theConfigSettings;
	readSettings.schemas.clear();

	// Parse XML
	//
	QDomDocument xml;
	QString parsingError;

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

		// Settings node
		//
		result &= applyCurSettingsProfile(curSettingsProfile, &readSettings);
	}

	// Error handling
	//
	if (result == false ||
			readSettings.errorMessage.isEmpty() == false)
	{
		QString completeErrorMessage = tr("Parsing configuration file error:\n%1").arg(readSettings.errorMessage);

		qDebug() << completeErrorMessage;
		QMessageBox::critical(m_parent, qApp->applicationName(), completeErrorMessage);

	}

	// Trace received params
	//
	theLogFile->writeMessage(tr("New configuration arrived"));
	theLogFile->writeMessage(tr("TUNS1 (id, ip, port): %1, %2, %3").arg(readSettings.serviceAddress.equipmentId()).arg(readSettings.serviceAddress.ip()).arg(readSettings.serviceAddress.port()));

	bool someFilesUpdated = false;

	QStringList updateInformation;

	QByteArray data;
	QString errorStr;

	// Get all schema details
	//
	{
		// Get SchemaDetails.pbuf file
		//

		QByteArray ba;
		QString fileName = "/" + theSettings.instanceStrId() + "/SchemaDetails.pbuf";
		bool ok = m_cfgLoaderThread->getFileBlocked(fileName, &ba, &parsingError);

		if (ok == false)
		{
			qDebug() << "ERROR: Cannot get " << fileName << ", " << parsingError;

			QMutexLocker l(&m_mutex);
			m_schemaDetailsSet.clear();
		}
		else
		{
			QMutexLocker l(&m_mutex);
			m_schemaDetailsSet.clear();

			m_schemaDetailsSet.Load(ba);
		}
	}

	std::vector<VFrame30::SchemaDetails> schemasDetails;
	{
		QMutexLocker l(&m_mutex);
		schemasDetails = m_schemaDetailsSet.schemasDetails();
	}

	for (const Builder::BuildFileInfo& buildFileInfo: buildFileInfoArray)
	{
		if (buildFileInfo.ID.isEmpty())
		{
			continue;
		}

		// Add MD5 record to a map, if it does not exist
		//

		if (m_filesMD5Map.find(buildFileInfo.ID) == m_filesMD5Map.end())
		{
			m_filesMD5Map[buildFileInfo.ID] = "";
		}

		// Get previous MD5
		//

		const QString& previousMD5 = m_filesMD5Map[buildFileInfo.ID];

		if (previousMD5 != buildFileInfo.md5)
		{
			m_filesMD5Map[buildFileInfo.ID] = buildFileInfo.md5;

			someFilesUpdated = true;

			updateInformation.push_back(tr("New configuration: file '%1' updated").arg(buildFileInfo.pathFileName));

			// Filters

			if (buildFileInfo.ID == CfgFileId::TUNING_FILTERS)
			{
				if (getFileBlockedById(buildFileInfo.ID, &data, &errorStr) == false)
				{
					QString completeErrorMessage = tr("ConfigController::getFileBlockedById: Get %1 file error:\n%2").arg(buildFileInfo.pathFileName).arg(errorStr);
					theLogFile->writeError(completeErrorMessage);
					QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
				}
				else
				{
					emit filtersArrived(data);
				}

			}

			// Signals

			if (buildFileInfo.ID == CfgFileId::TUNING_SIGNALS)
			{
				if (getFileBlockedById(buildFileInfo.ID, &data, &errorStr) == false)
				{
					QString completeErrorMessage = tr("ConfigController::getFileBlockedById: Get %1 file error:\n%2").arg(buildFileInfo.pathFileName).arg(errorStr);
					theLogFile->writeError(completeErrorMessage);
					QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
				}
				else
				{
					emit signalsArrived(data);
				}
			}

			if (buildFileInfo.ID == CfgFileId::TUNING_GLOBALSCRIPT)
			{
				QByteArray globalScriptData;

				if (getFileBlockedById(buildFileInfo.ID, &globalScriptData, &errorStr) == false)
				{
					QString completeErrorMessage = tr("ConfigController::getFileBlockedById: Get %1 file error:\n%2").arg(buildFileInfo.pathFileName).arg(errorStr);
					theLogFile->writeError(completeErrorMessage);
					QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
				}
				else
				{
					readSettings.scriptGlobal = globalScriptData;
				}
			}

			if (buildFileInfo.ID == CfgFileId::TUNING_CONFIGARRIVEDSCRIPT)
			{
				QByteArray configurationArrivedScriptData;

				if (getFileBlockedById(buildFileInfo.ID, &configurationArrivedScriptData, &errorStr) == false)
				{
					QString completeErrorMessage = tr("ConfigController::getFileBlockedById: Get %1 file error:\n%2").arg(buildFileInfo.pathFileName).arg(errorStr);
					theLogFile->writeError(completeErrorMessage);
					QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
				}
				else
				{
					readSettings.scriptConfigArrived = configurationArrivedScriptData;
				}
			}

			/*
			if (f.ID == CFG_FILE_ID_BEHAVIOR)
			{
				QByteArray behaviorData;

				if (getFileBlockedById(f.ID, &behaviorData, &errorStr) == false)
				{
					QString completeErrorMessage = tr("ConfigController::getFileBlockedById: Get %1 file error:\n%2").arg(f.pathFileName).arg(errorStr);
					theLogFile->writeError(completeErrorMessage);
					QMessageBox::critical(m_parent, tr("Error"), completeErrorMessage);
				}
				else
				{
					ClientBehaviorStorage behaviorStorage;

					QString errorCode;
					bool ok = behaviorStorage.load(behaviorData, &errorCode);
					if (ok == false)
					{
						Q_ASSERT(ok);
					}
				}
			}*/
		}

		// If this file is schema details - place the information about it

		for (auto si : schemasDetails)
		{
			if (buildFileInfo.ID == si.m_schemaId)
			{
				SchemaInfo s(si.m_schemaId, si.m_caption, si.tags());
				readSettings.schemas.push_back(s);
			}
		}
	}

	bool apperanceUpdated = false;

	if (theConfigSettings.clientSettings.appearanceChanged(readSettings.clientSettings)  == true ||
		theConfigSettings.scriptGlobal != readSettings.scriptGlobal ||
		theConfigSettings.scriptConfigArrived != readSettings.scriptConfigArrived ||
		theConfigSettings.schemas.size() != readSettings.schemas.size()
		)
	{

		updateInformation.push_back(tr("New configuration: appearance updated"));
		apperanceUpdated = true;
	}

	bool serversUpdated = false;

	if (theConfigSettings.clientSettings.connectionChanged(readSettings.clientSettings) == true)
	{
		updateInformation.push_back(tr("New configuration: connection parameters updated"));
		serversUpdated = true;
	}

	theConfigSettings = readSettings;

	for (const QString& str : updateInformation)
	{
		theLogFile->writeMessage(str);
	}

	// Emit signals to inform everybody about new configuration
	//

	if (serversUpdated == true)
	{

		emit tcpClientConfigurationArrived(theConfigSettings.serviceAddress.address(),
										   theConfigSettings.clientSettings.autoApply,
										   theConfigSettings.lmStatusFlagMode());
	}

	if (someFilesUpdated == true || apperanceUpdated == true)
	{

		// Modify logon mode

		if (theMainWindow->userManager()->isLoggedIn() == true)
		{
			theMainWindow->userManager()->logout();
		}

		theMainWindow->userManager()->setConfiguration(theConfigSettings.clientSettings.tuningLogin,
													   theConfigSettings.clientSettings.getUsersAccounts(),
													   theConfigSettings.clientSettings.loginPerOperation,
													   theConfigSettings.clientSettings.tuningSessionTimeout);

		emit configurationArrived();
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
	}

	return result;
}

bool ConfigController::xmlReadBuildInfoNode(const QDomNode& node, ConfigSettings* outSetting)
{
	if (outSetting == nullptr)
	{
		assert(outSetting);
		return false;
	}

	if (node.nodeName() != "BuildInfo")
	{
		assert(node.nodeName() == "BuildInfo");
		return false;
	}

	QDomElement element = node.toElement();

	outSetting->buildInfo.projectName = element.attribute("Project");
	outSetting->buildInfo.buildNo = element.attribute("ID").toInt();
	outSetting->buildInfo.configuration = element.attribute("Type");
	outSetting->buildInfo.date = element.attribute("Date");
	outSetting->buildInfo.changeset = element.attribute("Changeset").toInt();
	outSetting->buildInfo.user = element.attribute("User");
	outSetting->buildInfo.workstation = element.attribute("Workstation");

	return true;
}

bool ConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr)
	{
		assert(outSetting);
		return false;
	}

	if (softwareNode.nodeName() != "Software")
	{
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

bool ConfigController::applyCurSettingsProfile(std::shared_ptr<const SoftwareSettings> curSettingsProfile,
											   ConfigSettings* outSetting)
{
	if (outSetting == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	const TuningClientSettings* typedSettingsPtr = dynamic_cast<const TuningClientSettings*>(curSettingsProfile.get());

	if (typedSettingsPtr == nullptr)
	{
		outSetting->errorMessage += tr("Settings applying error!\n");
		return false;
	}

	outSetting->clientSettings = *typedSettingsPtr;

	outSetting->serviceAddress = ConfigConnection(outSetting->clientSettings.tuningServiceID,
														outSetting->clientSettings.tuningServiceIP,
														outSetting->clientSettings.tuningServicePort);

	return true;
}
