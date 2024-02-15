#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "ConfigController.h"
#include "../CommonLib/HostAddressPort.h"
#include "../OnlineLib/CfgServerLoader.h"
#include <QDomNode>


namespace ClientLib
{
	ConfigController::ConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address, ILogFile* logFile) :
		ConfigController{softwareInfo, address, address, logFile}
	{
	}

	ConfigController::ConfigController(const SoftwareInfo& softwareInfo, const HostAddressPort& address1, const HostAddressPort& address2, ILogFile* logFile) :
		QObject{nullptr},
		m_logFile{logFile, "ConfigController"},
		m_softwareInfo{softwareInfo}
	{
		m_appInstanceNo = acquireAppInstanceNo(E::valueToString(softwareInfo.softwareType()));

		m_logFile.writeMessage(QString("Assigned InstanceNo is %1").arg(m_appInstanceNo));

		// Create communication thread
		//
		m_cfgLoaderThread = std::make_unique<CfgLoaderThread>(m_softwareInfo,
															  m_appInstanceNo,
															  address1,
															  address2,
															  false,
															  std::make_shared<CircularLogger>(logFile, "CfgLoaderThread"));

		connect(m_cfgLoaderThread.get(), &CfgLoaderThread::signal_configurationReady, this, &ConfigController::slot_configurationReady);

		auto logFunc = [this](QString errMsg) { m_logFile.writeError(errMsg); };

		connect(m_cfgLoaderThread.get(), &CfgLoaderThread::signal_unknownClientID, this, &ConfigController::error);
		connect(m_cfgLoaderThread.get(), &CfgLoaderThread::signal_unknownClientID, logFunc);

		connect(m_cfgLoaderThread.get(), &CfgLoaderThread::signal_wrongClientHostname, this, &ConfigController::error);
		connect(m_cfgLoaderThread.get(), &CfgLoaderThread::signal_wrongClientHostname, logFunc);

		return;
	}

	ConfigController::~ConfigController()
	{
		releaseAppInstanceNo();
		m_cfgLoaderThread->quit();
	}

	void ConfigController::setConnectionParams(QString equipmentId, const HostAddressPort& address1, const HostAddressPort& address2)
	{
		m_softwareInfo.setEquipmentID(equipmentId);

		m_cfgLoaderThread->setConnectionParams(m_softwareInfo, address1, address2, true);

		return;
	}

	bool ConfigController::getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr)
	{
		QString error;
		bool result = m_cfgLoaderThread->getFileBlocked(pathFileName, fileData, &error);

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
		bool result = m_cfgLoaderThread->getFileBlockedByID(id, fileData, &error);

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
		return m_cfgLoaderThread->hasFileID(std::move(fileId));
	}

	Tcp::ConnectionState ConfigController::getConnectionState() const
	{
		return m_cfgLoaderThread->getConnectionState();
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

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/, const MonitorSettings& /*settings*/, const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/, const DiagnosticsSettings& /*settings*/, const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/, const TuningClientSettings& /*settings*/, const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/, const TestClientSettings& /*settings*/, const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	bool ConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& /*conf*/, const TestSuiteSettings& /*settings*/, const std::vector<OnlineLib::BuildFileInfo>& /*files*/)
	{
		// Reimplement in the derviced class
		//
		Q_ASSERT(false);
		return false;
	}

	int ConfigController::acquireAppInstanceNo(const QString& programName)
	{
		// Communication instance no
		//
		m_appInstanceSharedMemory.setKey(programName + "InstanceNo");

		int result = 0;

		if (bool ok = m_appInstanceSharedMemory.create(MaxInstanceCount * sizeof(qint64));
			ok == true)
		{
			// Shared memory just created, initialize it
			//
			std::lock_guard locker(m_appInstanceSharedMemory);

			qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
			Q_ASSERT(sharedData);

			std::fill(sharedData, sharedData + MaxInstanceCount, 0);

			sharedData[0] = QCoreApplication::instance()->applicationPid();

			result = 0;
		}
		else
		{
			if (m_appInstanceSharedMemory.error() == QSharedMemory::SharedMemoryError::AlreadyExists)
			{
				ok = m_appInstanceSharedMemory.attach();
			}

			if (ok == false)
			{
				m_logFile.writeAlert(tr("Cannot create or attach to shared memory to determine software instance no. Error: %1")
									 .arg(m_appInstanceSharedMemory.errorString()));

				// Set some Application Instance No, take random 127 high slots.
				//
				result = static_cast<int>(MaxInstanceCount - 1 - (QDateTime::currentMSecsSinceEpoch() & 0x7F));
			}
			else
			{
				// Get empty slot from the shared memory
				//
				Q_ASSERT(m_appInstanceSharedMemory.isAttached() == true);

				std::lock_guard locker(m_appInstanceSharedMemory);

				qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
				Q_ASSERT(sharedData);

				result = -1;

				for (int i = 0; i < MaxInstanceCount; i++)
				{
					if (sharedData[i] == 0)
					{
						// This is an empty slot, use it
						//
						sharedData[i] = qApp->applicationPid();
						result = i;
						break;
					}
				}

				if (result == -1)
				{
					m_logFile.writeAlert(tr("Cannot create or attach to shared memory to determine software instance no, there is no free slots."));

					// Set random Application Instance No, take random 127 high slots.
					//
					result = static_cast<int>(MaxInstanceCount - 1 - (QDateTime::currentMSecsSinceEpoch() & 0x7F));
				}
			}
		}

		return result;
	}

	void ConfigController::releaseAppInstanceNo()
	{
		// Release application instance slot
		//
		if (m_appInstanceNo < 0 || m_appInstanceNo >= MaxInstanceCount)
		{
			return;
		}

		Q_ASSERT(m_appInstanceSharedMemory.isAttached() == true);

		std::lock_guard locker(m_appInstanceSharedMemory);

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
		Q_ASSERT(sharedData);

		sharedData[m_appInstanceNo] = 0;

		return;
	}

	bool ConfigController::xmlReadBuildInfoNode(const QDomNode& buildInfoNode, ConfigurationInfo* out)
	{
		if (out == nullptr)
		{
			Q_ASSERT(out);
			return false;
		}

		if (buildInfoNode.nodeName() != "BuildInfo")
		{
			Q_ASSERT(buildInfoNode.nodeName() == "BuildInfo");
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

		if (int softwareType = softwareElement.attribute("Type").toInt();
			softwareType != m_softwareInfo.softwareType())
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

		m_cfgLoaderThread->start();
		m_cfgLoaderThread->enableDownloadConfiguration();

		return;
	}

	void ConfigController::slot_configurationReady(const QByteArray configurationXmlData,
												   const std::vector<OnlineLib::BuildFileInfo> buildFileInfoArray,
												   SessionParams /*sessionParams*/,
												   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
	{
		qDebug() << Q_FUNC_INFO;

		ConfigurationInfo conf{};

		// Parse mml
		//
		QString parsingError;
		int errorLine = 0;
		int errorColumn = 0;

		QDomDocument xml;
		bool result = xml.setContent(configurationXmlData, false, &parsingError, &errorLine, &errorColumn);

		if (result == false)
		{
			m_logFile.writeError(QString("Parse Configuration.xml error, %1, line %2, column %3")
								 .arg(parsingError)
								 .arg(errorLine)
								 .arg(errorColumn));
			return;
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
		auto callUpdateFunc = [this, &conf, &buildFileInfoArray] <typename T> (const T* settings)
			{
				if (settings != nullptr)
				{
					updateConfiguration(conf, *settings, buildFileInfoArray);
				}
			};

		switch (m_softwareInfo.softwareType())
		{
		case E::SoftwareType::Monitor:
			callUpdateFunc(dynamic_cast<const MonitorSettings*>(curSettingsProfile.get()));
			return;

		case E::SoftwareType::Diagnostics:
			callUpdateFunc(dynamic_cast<const DiagnosticsSettings*>(curSettingsProfile.get()));
			return;

		case E::SoftwareType::TuningClient:
			callUpdateFunc(dynamic_cast<const TuningClientSettings*>(curSettingsProfile.get()));
			return;

		case E::SoftwareType::TestClient:
			callUpdateFunc(dynamic_cast<const TestClientSettings*>(curSettingsProfile.get()));
			return;

		case E::SoftwareType::TestSuite:
			callUpdateFunc(dynamic_cast<const TestSuiteSettings*>(curSettingsProfile.get()));
			return;

		case E::SoftwareType::Unknown:
		case E::SoftwareType::BaseService:
		case E::SoftwareType::ConfigurationService:
		case E::SoftwareType::AppDataService:
		case E::SoftwareType::ArchiveService:
		case E::SoftwareType::TuningService:
		case E::SoftwareType::DiagDataService:
		case E::SoftwareType::Metrology:
		case E::SoftwareType::ServiceControlManager:
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

}
