#include "../lib/SoftwareSettings.h"
#include "MonitorConfigController.h"
#include "MonitorAppSettings.h"
#include "Globals.h"


MonitorConfigController::MonitorConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
	HasLogFile(logFile, "ConfigController"),
	m_softwareInfo(softwareInfo)
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
	m_cfgLoaderThread = new CfgLoaderThread(m_softwareInfo,
											m_appInstanceNo,
											address1,
											address2,
											false,
											std::make_shared<CircularLogger>(logFile, "CfgLoaderThread"));

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &MonitorConfigController::slot_configurationReady);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_unknownClientID, this, &MonitorConfigController::unknownClient);
	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_unknownClientID, [this](){this->writeError(tr("Unknown client %1").arg(MonitorAppSettings::instance().equipmentId()));});

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_wrongClientHostname, this, &MonitorConfigController::wrongClientHostname);

	return;
}

MonitorConfigController::~MonitorConfigController()
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
	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
}

void MonitorConfigController::setConnectionParams(QString equipmentId, HostAddressPort address1, HostAddressPort address2)
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

bool MonitorConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
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

bool MonitorConfigController::getFile(const QString& pathFileName, QByteArray* fileData)
{
	Q_UNUSED(pathFileName);
	Q_UNUSED(fileData);

	// To do
	//
	Q_ASSERT(false);
	return false;
}

bool MonitorConfigController::getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr)
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
	else
	{
		writeMessage(tr("getFileBlockedById('%1') Success").arg(id));
	}

	return result;
}

bool MonitorConfigController::getFileById(const QString& id, QByteArray* fileData)
{
	Q_UNUSED(id);
	Q_UNUSED(fileData);

	// To do
	//
	Q_ASSERT(false);
	return false;
}

bool MonitorConfigController::hasFileId(QString fileId) const
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread != nullptr);
		return false;
	}

	return m_cfgLoaderThread->hasFileID(std::move(fileId));
}

Tcp::ConnectionState MonitorConfigController::getConnectionState() const
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

const SoftwareInfo& MonitorConfigController::softwareInfo() const
{
	return m_softwareInfo;
}

void MonitorConfigController::start()
{
	if (m_cfgLoaderThread == nullptr)
	{
		Q_ASSERT(m_cfgLoaderThread);
		return;
	}

	writeMessage(tr("MonitorConfigController::start()"));

	m_cfgLoaderThread->start();
	m_cfgLoaderThread->enableDownloadConfiguration();

	return;
}

void MonitorConfigController::slot_configurationReady(const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> curSettingsProfile)
{
	Q_UNUSED(buildFileInfoArray);
	Q_UNUSED(sessionParams);

	qDebug() << "MonitorConfigThread::slot_configurationReady";

	// Get GlobalScript.js file
	//
	auto getScriptFunc = [cfgLoaderThread = m_cfgLoaderThread](QString scriptFileName) -> QString
		{
			QString parsingError;
			QByteArray ba;

			if (bool ok = cfgLoaderThread->getFileBlocked(scriptFileName, &ba, &parsingError);
				ok == true)
			{
				return QString{ba};
			}
			else
			{
				return {};
			}
		};

	// Get image file
	//
	auto getImageFunc = [cfgLoaderThread = m_cfgLoaderThread](QString fileId) -> QImage
		{
			QString parsingError;
			QByteArray ba;

			if (bool ok = cfgLoaderThread->getFileBlockedByID(fileId, &ba, &parsingError);
				ok == true)
			{
				return QImage::fromData(ba);
			}
			else
			{
				return {};
			}
		};

	ConfigSettings readSettings;

	readSettings.globalScript = getScriptFunc("/" + MonitorAppSettings::instance().equipmentId() + "/GlobalScript.js");
	readSettings.logoImage = getImageFunc(CfgFileId::LOGO);
	readSettings.onConfigurationArrivedScript = getScriptFunc("/" + MonitorAppSettings::instance().equipmentId() + "/OnConfigurationArrived.js");

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
			result &= applyCurSettingsProfile(curSettingsProfile, &readSettings);
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

	// Get tuning signal files
	//
	theTuningSignals.reset();

	if (readSettings.tuningEnabled == true)
	{
		QByteArray data;
		QString errorString;

		bool result = getFileBlockedById(CfgFileId::TUNING_SIGNALS, &data, &errorString);

		if (result == false)
		{
			readSettings.errorMessage += errorString + QStringLiteral("\n");
		}
		else
		{
			theTuningSignals.load(data);
		}
	}

	// Get all schema details
	//
	{
		// Get SchemaDetails.pbuf file
		//
		QString parsingError;

		QByteArray ba;
		QString fileName = "/" + MonitorAppSettings::instance().equipmentId() + QStringLiteral("/SchemaDetails.pbuf");
		bool ok = m_cfgLoaderThread->getFileBlocked(fileName, &ba, &parsingError);

		if (ok == false)
		{
			qDebug() << "ERROR: Cannot get " << fileName << ", " << parsingError;

			QWriteLocker locker(&m_schemaDetailsLock);
			m_schemaDetailsSet.clear();
		}
		else
		{
			QWriteLocker locker(&m_schemaDetailsLock);
			m_schemaDetailsSet.clear();
			m_schemaDetailsSet.Load(ba);
		}
	}

	// Trace received params
	//
	qDebug() << "New configuration arrived";
	writeMessage(tr("New configuration arrived: StartSchemaID %1").arg(readSettings.startSchemaId));

	qDebug() << "StartSchemaID: " << readSettings.startSchemaId;

	// --
	//
	writeMessage(tr("AppDatService(s): %1").arg(readSettings.appDataServices.size()));
	qDebug() << "AppDatService(s):";

	for (const MonitorSettings::AppDataService& service : readSettings.appDataServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		writeMessage(tr("Service: id, address: %1, %2").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	writeMessage(tr("AppDataRealTimeService(s): %1").arg(readSettings.appDataRealTimeServices.size()));
	qDebug() << "AppDataRealTimeService(s):";

	for (const MonitorSettings::AppDataService& service : readSettings.appDataRealTimeServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		writeMessage(tr("Service: id, address: %1, %2").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	writeMessage(tr("ArchiveService(s): %1").arg(readSettings.archiveServices.size()));
	qDebug() << "ArchiveService(s):";
	for (const MonitorSettings::ArchiveService& service : readSettings.archiveServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		writeMessage(tr("Service: id, address: %1, %2").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	writeMessage(QString("TuningEnabled = %1").arg(readSettings.tuningEnabled));
	if (readSettings.tuningEnabled == true)
	{
		for (const MonitorSettings::TuningService& ts : readSettings.tuningServices)
		{
			writeMessage(tr("TuningService (id, ip, port): %1, %2, %3").arg(ts.equipmentId).arg(ts.clientRequestIP).arg(ts.clientRequestPort));
			writeMessage(tr("TuningSources: %1").arg(ts.drivenSources.join(", ")));
		}
		writeMessage(tr("TuningUserAccounts: %1").arg(readSettings.tuningUserAccounts.join(", ")));
		writeMessage(tr("TuningSessionTimeout: %1").arg(readSettings.tuningSessionTimeout));
	}

	if (readSettings.errorMessage.isEmpty() == false)
	{
		writeError(tr("Error: %1").arg(readSettings.errorMessage));
	}

	// New setpoints
	//
	{
		QByteArray data;
		QString errorString;

		if (bool result = getFileBlockedById(CfgFileId::COMPARATOR_SET, &data, &errorString);
			result == false)
		{
			readSettings.errorMessage += errorString + QStringLiteral("\n");
		}
		else
		{
			ComparatorSet setpoints;

			if (bool readOk = setpoints.serializeFrom(data);
				readOk == false)
			{
				readSettings.errorMessage += tr("Serialize set point list file error") + QStringLiteral("\n");
			}
			else
			{
				m_setpoints = std::move(setpoints);
			}
		}
	}

	// Monitor Behavior
	//
	{
		QByteArray data;
		QString errorString;

		if (bool result = getFileBlockedById(CfgFileId::CLIENT_BEHAVIOR, &data, &errorString);
			result == false)
		{
			readSettings.errorMessage += errorString + QStringLiteral("\n");
		}
		else
		{
			ClientBehaviorStorage behavior;
			behavior.clear();

			bool ok = behavior.load(data, &errorString);

			if (ok == false)
			{
				readSettings.errorMessage += tr("Read/parse Behavior file errror: ") + errorString + QStringLiteral("\n");
			}
			else
			{
				std::vector<std::shared_ptr<MonitorBehavior>> mb = behavior.monitorBehaviors();

				if (mb.empty() == false)
				{
					readSettings.monitorBeahvior = std::move(*mb[0]);
				}
			}
		}
	}

	// --
	//
	{
		QWriteLocker locker(&m_confugurationLock);
		readSettings.configurationId = s_configurationIdCounter++;
		m_configuration = readSettings;		// Cannot move readSettings here as it is used later for `emit configurationArrived(readSettings)`
	}

	// Emit signal to inform everybody about new configuration
	//
	emit configurationArrived(readSettings);
	emit configurationUpdate();

	return;
}

bool MonitorConfigController::xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigSettings* outSetting)
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

bool MonitorConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting)
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

	if (MonitorAppSettings::instance().equipmentId() != appEquipmentId)
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

bool MonitorConfigController::applyCurSettingsProfile(std::shared_ptr<const SoftwareSettings> curSettingsProfile, ConfigSettings* outSetting)
{
	if (curSettingsProfile == nullptr)
	{
		Q_ASSERT(curSettingsProfile);
		return false;
	}

	if (outSetting == nullptr)
	{
		Q_ASSERT(outSetting);
		return false;
	}

	const MonitorSettings* ms = dynamic_cast<const MonitorSettings*>(curSettingsProfile.get());

	if (ms == nullptr)
	{
		Q_ASSERT(ms);
		return false;
	}

	// --
	//
	outSetting->startSchemaId = ms->startSchemaId;

	// --
	//
	outSetting->appDataServices = ms->appDataServices;
	outSetting->appDataRealTimeServices = ms->appDataServices;
	outSetting->archiveServices = ms->archiveServices;

	//  --
	//
	outSetting->tuningEnabled = ms->tuningEnabled;

	if (ms->tuningEnabled == true)
	{
		outSetting->tuningServices = ms->tuningServices;
		outSetting->tuningLogin = ms->tuningLogin;
		outSetting->tuningUserAccounts = ms->getUsersAccounts();
		outSetting->tuningSessionTimeout = ms->tuningSessionTimeout;
	}
	else
	{
		// tuning disabled
		//
		outSetting->tuningServices.clear();
		outSetting->tuningLogin = false;
		outSetting->tuningUserAccounts.clear();
		outSetting->tuningSessionTimeout = 0;
	}

	return true;
}

VFrame30::SchemaDetailsSet MonitorConfigController::schemasDetailsSet() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet;
}

std::vector<VFrame30::SchemaDetails> MonitorConfigController::schemasDetails() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasDetails();
}

std::set<QString> MonitorConfigController::schemaAppSignals(const QString& schemaId)
{
	QReadLocker l(&m_schemaDetailsLock);

	std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetailsSet.schemaDetails(schemaId);
	if (details == nullptr)
	{
		return {};
	}

	return details->m_signals;
}

QStringList MonitorConfigController::schemasByAppSignalId(const QString& appSignalId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasByAppSignalId(appSignalId);
}

QStringList MonitorConfigController::schemasByLoopbackId(const QString& loopbackId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasByLoopbackId(loopbackId);
}

int MonitorConfigController::configurationId() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.configurationId;
}

ConfigSettings MonitorConfigController::configuration() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration;
}

QString MonitorConfigController::configurationStartSchemaId() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.startSchemaId;
}

int MonitorConfigController::schemaCount() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCount();
}

QString MonitorConfigController::schemaCaptionById(const QString& schemaId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCaptionById(schemaId);
}

QString MonitorConfigController::schemaCaptionByIndex(int schemaIndex) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCaptionByIndex(schemaIndex);
}

QString MonitorConfigController::schemaIdByIndex(int schemaIndex) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaIdByIndex(schemaIndex);
}

std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> MonitorConfigController::trendSchemaItems() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.trendIndicators();
}

const ComparatorSet& MonitorConfigController::setpoints() const
{
	return m_setpoints;
}
