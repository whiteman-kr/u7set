#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include <ClientLib/ConfigController.h>
#include <CommonLib/HostAddressPort.h>

#include <QCoreApplication>
#include <QDir>
#include <QDomNode>
#include <QStandardPaths>


namespace ClientLib
{
	ConfigController::ConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address, ILogFile* logFile) :
		ConfigController{softwareInfo, address, address, logFile}
	{
	}

	ConfigController::ConfigController(const SoftwareInfo& softwareInfo,
									   const HostAddressPort& address1,
									   const HostAddressPort& address2,
									   ILogFile* logFile) :
		QObject{nullptr},
		m_logFile{logFile, "ConfigController"},
		m_softwareInfo{softwareInfo}
	{
		qDebug() << "ConfigController::ConfigController";

		m_appInstanceNo = acquireAppInstanceNo(E::valueToString(softwareInfo.softwareType()));

		m_logFile.writeMessage(QString("Assigned InstanceNo is %1").arg(m_appInstanceNo));

		// Create communication thread
		//
		m_grpcCfgLoaderThread = std::make_unique<GrpcCfgLoaderThread>(m_softwareInfo,
																	  m_appInstanceNo,
																	  address1,
																	  address2,
																	  std::make_shared<CircularLogger>(logFile, "GrpcCfgLoaderThread"));

		connect(m_grpcCfgLoaderThread.get(),
				&GrpcCfgLoaderThread::signal_configurationReady,
				this,
				&ConfigController::slot_configurationReady);

		auto logFunc = [this](QString errMsg)
		{
			m_logFile.writeError(errMsg);
		};

		connect(m_grpcCfgLoaderThread.get(), &GrpcCfgLoaderThread::signal_unknownClientID, this, &ConfigController::error);
		connect(m_grpcCfgLoaderThread.get(), &GrpcCfgLoaderThread::signal_unknownClientID, logFunc);

		connect(m_grpcCfgLoaderThread.get(), &GrpcCfgLoaderThread::signal_wrongClientHostname, this, &ConfigController::error);
		connect(m_grpcCfgLoaderThread.get(), &GrpcCfgLoaderThread::signal_wrongClientHostname, logFunc);

		return;
	}

	ConfigController::~ConfigController()
	{
		qDebug() << "ConfigController::~ConfigController()";

		releaseAppInstanceNo();

		qDebug() << "ConfigController::~ConfigController";
		// m_cfgLoaderThread->quitAndWait();
	}

	void ConfigController::setConnectionParams(QString equipmentId, const HostAddressPort& address1, const HostAddressPort& address2)
	{
		m_softwareInfo.setEquipmentID(equipmentId);

		m_grpcCfgLoaderThread->setConnectionParams(m_softwareInfo, address1, address2);

		return;
	}

	bool ConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
	{
		QString error;
		bool result = m_grpcCfgLoaderThread->getFileBlocked(pathFileName, fileData, &error);

		if (result == false)
		{
			m_logFile.writeError(tr("getFileBlocked() Can't get file %1, error %2").arg(pathFileName).arg(error));
		}
		else
		{
			m_logFile.writeMessage(tr("getFileBlocked('%1') Success").arg(pathFileName));
		}

		if (errorStr != nullptr)
		{
			*errorStr = error;
		}

		return result;
	}

	bool ConfigController::getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr)
	{
		QString error;
		bool result = m_grpcCfgLoaderThread->getFileBlockedByID(id, fileData, &error);

		if (result == false)
		{
			m_logFile.writeError(tr("getFileBlockedById() Can't get fileid %1, error %1").arg(id).arg(error));
		}
		else
		{
			m_logFile.writeMessage(tr("getFileBlockedById('%1') Success").arg(id));
		}

		if (errorStr != nullptr)
		{
			*errorStr = error;
		}

		return result;
	}

	bool ConfigController::hasFileId(QString fileId) const
	{
		return m_grpcCfgLoaderThread->hasFileID(std::move(fileId));
	}

	Tcp::ConnectionState ConfigController::getConnectionState() const
	{
		return m_grpcCfgLoaderThread->getConnectionState();
	}

	const SoftwareInfo& ConfigController::softwareInfo() const
	{
		return m_softwareInfo;
	}

	ILogFile* ConfigController::logFile()
	{
		return m_logFile.logFile();
	}

	ILogFile* ConfigController::logFile() const
	{
		return m_logFile.logFile();
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/,
											   const MonitorSettings& /*settings*/,
											   const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/,
											   const DiagnosticsSettings& /*settings*/,
											   const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/,
											   const TuningClientSettings& /*settings*/,
											   const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/,
											   const TestClientSettings& /*settings*/,
											   const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/,
											   const TestSuiteSettings& /*settings*/,
											   const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	int ConfigController::acquireAppInstanceNo(const QString& programName)
	{
		auto acquireFallbackInstanceNo = [&](const QString& reason)
		{
			m_appInstanceNoIsFallback = true;

			const int fallbackInstanceNo = MaxInstanceCount + static_cast<int>(QDateTime::currentMSecsSinceEpoch() & 0xFF);

			m_logFile.writeAlert(
				tr("Using fallback instance no %1 for %2. Reason: %3").arg(fallbackInstanceNo).arg(programName).arg(reason));

			qDebug() << "ConfigController::acquireAppInstanceNo(): Fallback instance no:" << fallbackInstanceNo << ", reason:" << reason;

			return fallbackInstanceNo;
		};

		auto lockErrorToString = [](QLockFile& lockFile)
		{
			switch (lockFile.error())
			{
			case QLockFile::NoError:
				return QStringLiteral("NoError");

			case QLockFile::LockFailedError:
				{
					qint64 pid = 0;
					QString hostName;
					QString appName;

					if (lockFile.getLockInfo(&pid, &hostName, &appName))
					{
						return QStringLiteral("LockFailedError; owner pid=%1 host=%2 app=%3").arg(pid).arg(hostName).arg(appName);
					}

					return QStringLiteral("LockFailedError");
				}

			case QLockFile::PermissionError:
				return QStringLiteral("PermissionError");

			case QLockFile::UnknownError:
				return QStringLiteral("UnknownError");
			}

			return QStringLiteral("UnexpectedLockError");
		};

		const QString tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		if (tempLocation.isEmpty())
		{
			return acquireFallbackInstanceNo(tr("TempLocation is empty"));
		}

		const QString lockDirPath = QDir(tempLocation).filePath(QStringLiteral("ClientLibInstanceNoLocks"));
		if (QDir().mkpath(lockDirPath) == false && QDir(lockDirPath).exists() == false)
		{
			return acquireFallbackInstanceNo(tr("Cannot create lock directory %1").arg(lockDirPath));
		}

		auto getInstanceLockFilePath = [&lockDirPath, &programName](int instanceNo)
		{
			return QDir(lockDirPath).filePath(QStringLiteral("%1-InstanceNo-%2.lock").arg(programName).arg(instanceNo));
		};

		int occupiedSlots = 0;
		QString firstLockFailureDetails;

		for (int i = 0; i < MaxInstanceCount; i++)
		{
			auto lockFile = std::make_unique<QLockFile>(getInstanceLockFilePath(i));
			lockFile->setStaleLockTime(0);

			if (lockFile->tryLock())
			{
				m_appInstanceNoIsFallback = false;
				m_appInstanceLockFile = std::move(lockFile);

				qDebug() << "ConfigController::acquireAppInstanceNo(): Instance lock acquired:" << i
						 << ", thread: " << QThread::currentThread();

				return i;
			}

			const QString lockFailureDetails = lockErrorToString(*lockFile);

			if (lockFile->error() == QLockFile::LockFailedError)
			{
				occupiedSlots++;

				if (firstLockFailureDetails.isEmpty())
				{
					firstLockFailureDetails = tr("slot %1 file %2: %3").arg(i).arg(lockFile->fileName()).arg(lockFailureDetails);
				}

				continue;
			}

			return acquireFallbackInstanceNo(
				tr("Cannot lock slot %1 file %2: %3").arg(i).arg(lockFile->fileName()).arg(lockFailureDetails));
		}

		return acquireFallbackInstanceNo(
			tr("All %1 slots are occupied in %2. First lock detail: %3").arg(occupiedSlots).arg(lockDirPath).arg(firstLockFailureDetails));
	}

	void ConfigController::releaseAppInstanceNo()
	{
		qDebug() << "ConfigController::releaseAppInstanceNo(): " << m_appInstanceNo << ", thread: " << QThread::currentThread()
				 << ", hasLockFile: " << (m_appInstanceLockFile != nullptr);

		// Release application instance slot
		//
		if (m_appInstanceNo < 0 || m_appInstanceLockFile == nullptr)
		{
			return;
		}

		if (m_appInstanceNoIsFallback == true)
		{
			return;
		}

		if (m_appInstanceLockFile == nullptr)
		{
			m_logFile.writeAlert(tr("Cannot release application instance no %1 because the lock file is missing.").arg(m_appInstanceNo));
			return;
		}

		m_appInstanceLockFile->unlock();
		m_appInstanceLockFile.reset();

		return;
	}

	bool ConfigController::xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigurationInfo* out)
	{
		if (out == nullptr)
		{
			Q_ASSERT(out);
			return false;
		}

		if (buildInfoNode.nodeName() != XmlElement::BUILD_INFO)
		{
			Q_ASSERT(buildInfoNode.nodeName() == XmlElement::BUILD_INFO);
			return false;
		}

		QDomElement element = buildInfoNode.toElement();
		out->buildNo = element.attribute(QLatin1String("ID")).toInt();
		out->project = element.attribute(QLatin1String("Project"));

		return true;
	}

	bool ConfigController::xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigurationInfo* out)
	{
		if (out == nullptr)
		{
			Q_ASSERT(out);
			return false;
		}

		if (softwareNode.nodeName() != "Software")
		{
			Q_ASSERT(softwareNode.nodeName() == "Software");
			return false;
		}

		QDomElement softwareElement = softwareNode.toElement();

		out->softwareEquipmentId = softwareElement.attribute(EquipmentPropNames::EQUIPMENT_ID);

		if (int softwareType = softwareElement.attribute("Type").toInt(); softwareType != m_softwareInfo.softwareType())
		{
			// The received file has different type then expected,
			//
			m_logFile.writeError(QString("Parse configuarion error, received wrong softaware type, expected %1, received %2")
									 .arg(softwareType)
									 .arg(m_softwareInfo.softwareType()));
			return false;
		}

		return true;
	}

	void ConfigController::start()
	{
		m_logFile.writeMessage(tr("MonitorConfigController::start()"));
		m_grpcCfgLoaderThread->start();
		return;
	}

	void ConfigController::slot_configurationReady(const QByteArray configurationXmlData,
												   const std::vector<OnlineLib::BuildFileInfo> buildFileInfoArray,
												   SessionParams /*sessionParams*/,
												   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
	{
		qDebug() << "ConfigController::slot_configurationReady()";

		ConfigurationInfo conf{};
		bool result = true;

		// Parse mml
		//
		QDomDocument xml;

		QDomDocument::ParseResult pr = xml.setContent(configurationXmlData);

		if (pr.errorMessage.isEmpty() == false)
		{
			m_logFile.writeError(QString("Parse Configuration.xml error, %1, line %2, column %3")
									 .arg(pr.errorMessage)
									 .arg(pr.errorLine)
									 .arg(pr.errorColumn));
			return;
		}
		else
		{
			// Get <Configuration>
			//
			QDomElement configElement = xml.documentElement();

			// BuildInfo node
			//
			QDomNodeList buildInfoNodes = configElement.elementsByTagName(XmlElement::BUILD_INFO);

			if (buildInfoNodes.size() != 1)
			{
				m_logFile.writeError("Parse Configuration.xml error, node BuildInfo is missing.");
				return;
			}
			else
			{
				result &= xmlReadBuildInfoNode(buildInfoNodes.item(0), &conf);
			}

			// Software node
			//
			QDomNodeList softwareNodes = configElement.elementsByTagName("Software");
			if (softwareNodes.size() != 1)
			{
				m_logFile.writeError("Parse Configuration.xml error, node Software is missing.");
				return;
			}
			else
			{
				result &= xmlReadSoftwareNode(softwareNodes.item(0), &conf);
			}

			if (result == false)
			{
				return;
			}
		}


		// Call specific updateConfiguration
		//
		auto callUpdateFunc = [this, &conf, &buildFileInfoArray]<typename T>(const T* settings)
		{
			if (settings != nullptr)
			{
				updateConfiguration(conf, *settings, buildFileInfoArray);
			}
		};

		switch (m_softwareInfo.softwareType())
		{
			using enum E::SoftwareType;

		case Monitor:
			callUpdateFunc(dynamic_cast<const MonitorSettings*>(curSettingsProfile.get()));
			return;

		case Diagnostics:
			callUpdateFunc(dynamic_cast<const DiagnosticsSettings*>(curSettingsProfile.get()));
			return;

		case TuningClient:
			callUpdateFunc(dynamic_cast<const TuningClientSettings*>(curSettingsProfile.get()));
			return;

		case TestClient:
			callUpdateFunc(dynamic_cast<const TestClientSettings*>(curSettingsProfile.get()));
			return;

		case TestSuite:
			callUpdateFunc(dynamic_cast<const TestSuiteSettings*>(curSettingsProfile.get()));
			return;

		case AdsBridge:
		case AppDataService:
		case ArchiveService:
		case BaseService:
		case ConfigurationService:
		case DiagDataService:
		case GatewayService:
		case Metrology:
		case ServiceControlManager:
		case TuningService:
		case Unknown:
			// Not supported by this class
			//
			Q_ASSERT(false);
			return;
		}

		// Implement appropriate updateConfiguration
		//
		Q_ASSERT(false);

		return;
	}

	int ConfigController::appInstanceNo() const
	{
		return m_appInstanceNo;
	}

} // namespace ClientLib
