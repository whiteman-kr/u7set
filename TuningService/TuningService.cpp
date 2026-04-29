#include "../UtilsLib/WUtils.h"
#include "TuningService.h"


namespace Tuning
{
	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningServiceWorker::TuningServiceWorker(const SoftwareInfo& softwareInfo,
											 const QString& serviceName,
											 int argc,
											 char** argv,
											 CircularLoggerShared logger,
											 CircularLoggerShared tuningLog) :
		ServiceWorker(softwareInfo, serviceName, argc, argv, logger, "TuningServiceWorker"),
		m_tuningLog(tuningLog)
	{
	}

	TuningServiceWorker::TuningServiceWorker(const TuningServiceWorker* worker) :
		ServiceWorker(worker),
		m_tuningLog(worker->tuningLog())
	{
	}

	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}

	ServiceWorker* TuningServiceWorker::createInstance() const
	{
		TuningServiceWorker* newInstance = new TuningServiceWorker(this);
		return newInstance;
	}

	void TuningServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
	{
		QMutexLocker l(&m_startStopMutex);

		QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::TuningService, m_serviceSettings);

		serviceInfo.set_settingsxml(xmlString.toStdString());

		if (m_tcpTuningServerThread != nullptr)
		{
			m_tcpTuningServerThread->getClientsList(&serviceInfo);
		}

		m_tuningSources.getTuningSourcesInfo(&serviceInfo);

		int srcCount = serviceInfo.tuningsourcesinfostate_size();

		for(int i = 0; i < srcCount; i++)
		{
			const Network::DataSourceInfo& dsi = serviceInfo.tuningsourcesinfostate(i).info();

			TuningSourceThreadShared thread = getValueOrNullptr(m_sourceThreads, QString::fromStdString(dsi.moduleequipmentid()));

			TEST_PTR_CONTINUE(thread);

			thread->getSourceState(serviceInfo.mutable_tuningsourcesinfostate(i)->mutable_state());
		}
	}

	void TuningServiceWorker::initServiceSpecificCmdLineArgs()
	{
		addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");

		addValueCmdLineArg(CmdLineArg::CFG_IP1, SoftwareSetting::CFG_SERVICE_IP1,
								QString("IP-address of first Configuration Service (default port - %1).").
											arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST), "ip[:port]");
		addValueCmdLineArg(CmdLineArg::CFG_IP2, SoftwareSetting::CFG_SERVICE_IP2,
								QString("IP-address of second Configuration Service (default port - %1).").
											arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST), "ip[:port]");
	}

	void TuningServiceWorker::loadServiceSpecificSettings()
	{
		DEBUG_LOG_MSG(logger(), "");
		DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
		DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
		DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
		DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));
		DEBUG_LOG_MSG(logger(), "");
	}

	void TuningServiceWorker::clear()
	{
		m_tuningSources.clear();
	}

	const TuningClientContext* TuningServiceWorker::getClientContext(QString clientID) const
	{
		return m_clientContextMap.getClientContext(clientID);
	}

	const TuningClientContext* TuningServiceWorker::getClientContext(const std::string& clientID) const
	{
		return m_clientContextMap.getClientContext(QString::fromStdString(clientID));
	}

	TuningSourceThreadShared TuningServiceWorker::getTuningSourceThread(quint32 sourceIP)
	{
		auto it = m_ip2sourceThread.find(sourceIP);

		if (it == m_ip2sourceThread.end())
		{
			return nullptr;
		}

		return it->second;
	}

	TuningSourceThreadShared TuningServiceWorker::getTuningSourceThread(const QString& sourceID)
	{
		auto it = m_sourceThreads.find(sourceID);

		if (it == m_sourceThreads.end())
		{
			return nullptr;
		}

		return it->second;
	}

	void TuningServiceWorker::getAllClientContexts(QVector<const TuningClientContext*>& clientContexts)
	{
		m_clientContextMap.getAllClientContexts(clientContexts);
	}

	bool TuningServiceWorker::singleLmControl() const
	{
		return m_serviceSettings.singleLmControl;
	}

	// called from TcpTuningServer thread!!!
	//
	E::NetworkError TuningServiceWorker::changeControlledTuningSource(const QString& tuningSourceEquipmentID,
												bool activateControl,
												QString* controlledTuningSource,
												bool* controlIsActive)
	{
		if (controlledTuningSource == nullptr || controlIsActive == nullptr)
		{
			return E::NetworkError::InternalError;
		}

		if (m_serviceSettings.singleLmControl == false)
		{
			controlledTuningSource->clear();
			*controlIsActive = false;
			return E::NetworkError::SingleLmControlDisabled;
		}

		AUTO_LOCK(m_startStopMutex);							// !!!!

		if (m_tuningSources.getSourceByID(tuningSourceEquipmentID) == nullptr)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return E::NetworkError::UnknownTuningSourceID;
		}

		stopSourcesListenerThreads();

		stopTuningSourceThreads();

		if (activateControl == false)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return E::NetworkError::Success;
		}

		bool result = runTuningSourceThread(true, tuningSourceEquipmentID);

		if (result == false)
		{
			*controlledTuningSource = tuningSourceEquipmentID;
			*controlIsActive = false;
			return E::NetworkError::InternalError;
		}

		runSourcesListenerThreads();

		*controlledTuningSource = tuningSourceEquipmentID;
		*controlIsActive = true;

		return E::NetworkError::Success;
	}

	bool TuningServiceWorker::clientIsConnected(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		{
			std::lock_guard lg(m_activeClientInfoMutex);

			if (m_serviceSettings.singleLmControl == true)
			{
				if (m_activeClientInfo.equipmentID().isEmpty() == true)
				{
					m_activeClientInfo = softwareInfo;
					m_activeClientIP = clientIP;
				}
			}
			else
			{
				m_activeClientInfo.clear();
				m_activeClientIP.clear();
			}
		}

		return true;
	}

	bool TuningServiceWorker::clientIsDisconnected(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		{
			std::lock_guard lg(m_activeClientInfoMutex);

			if (m_serviceSettings.singleLmControl == true)
			{
				if (m_activeClientInfo.equipmentID() == softwareInfo.equipmentID() &&
					m_activeClientIP == clientIP)
				{
					m_activeClientInfo.clear();
					m_activeClientIP.clear();
				}
			}
			else
			{
				m_activeClientInfo.clear();
				m_activeClientIP.clear();
			}
		}

		return true;
	}

	bool TuningServiceWorker::setActiveClient(const SoftwareInfo& softwareInfo, const QString& clientIP)
	{
		if (softwareInfo.softwareType() == E::SoftwareType::ServiceControlManager)
		{
			return true;
		}

		{
			std::lock_guard lg(m_activeClientInfoMutex);

			if (m_serviceSettings.singleLmControl == true)
			{
				m_activeClientInfo = softwareInfo;
				m_activeClientIP = clientIP;
			}
			else
			{
				m_activeClientInfo.clear();
				m_activeClientIP.clear();
			}
		}

		return true;
	}

	QString TuningServiceWorker::activeClientID() const
	{
		QString clientID;

		std::lock_guard lg(m_activeClientInfoMutex);

		clientID = m_activeClientInfo.equipmentID();

		return clientID;
	}

	QString TuningServiceWorker::activeClientIP() const
	{
		QString clientIP;

		std::lock_guard lg(m_activeClientInfoMutex);

		clientIP = m_activeClientIP;

		return clientIP;
	}

	bool TuningServiceWorker::isControlled(const QString& lmEquipmentID, const QString& lanEquipmentID) const
	{
		return m_controlledLans.contains({ lmEquipmentID, lanEquipmentID });
	}

	void TuningServiceWorker::logTuningPacket(bool request,
											  Fotip::OpCode opCode,
											  quint16 rupNumerator,
											  quint64 fotipNumerator)
	{
		if (m_tuningPacketLog == nullptr)
		{
			return;
		}

		QString opCodeStr;

		switch(opCode)
		{
		case Fotip::OpCode::Read:
			opCodeStr = "READ&nbsp;";
			break;

		case Fotip::OpCode::Write:
			opCodeStr = "WRITE";
			break;

		case Fotip::OpCode::Apply:
			opCodeStr = "APPLY";
			break;

		default:
			//Q_ASSERT(false);
			opCodeStr = QString("Unknown opCode = %1").arg(TO_INT(opCode));
		};

		LOG_MSG(m_tuningPacketLog, QString("%1 %2 %3 %4").
				arg(rupNumerator, sizeof(rupNumerator) * 2, 16, Latin1Char::ZERO).
				arg(request == true ? "request" : "reply&nbsp;&nbsp;" ).
				arg(opCodeStr).
				arg(fotipNumerator, sizeof(fotipNumerator) * 2, 16, Latin1Char::ZERO));
	}

	E::SecurityLevel TuningServiceWorker::securityLevel() const
	{
		return m_serviceSettings.securityLevel;
	}

	void TuningServiceWorker::registerSignalsStateChangesQueue(const QString& clientEquipmentID,
														qint64 tcpConnectionID)
	{
		TuningClientContext* clientContext = m_clientContextMap.getClientContext(clientEquipmentID);

		if (clientContext != nullptr)
		{
			clientContext->registerStateChangesQueue(tcpConnectionID);
		}
	}

	void TuningServiceWorker::unregisterSignalsStateChangesQueue(const QString& clientEquipmentID, qint64 tcpConnectionID)
	{
		TuningClientContext* clientContext = m_clientContextMap.getClientContext(clientEquipmentID);

		if (clientContext != nullptr)
		{
			clientContext->unregisterStateChangesQueue(tcpConnectionID);
		}
	}

	void TuningServiceWorker::pushSignalStateChange(const TuningSignal::State& state)
	{
		m_clientContextMap.pushSignalStateChange(state);
	}

	TuningSignalsChangesQueue* TuningServiceWorker::getSignalChangesQueue(const QString& clientEquipmentID,
																		  qint64 tcpConnectionID)
	{
		TuningClientContext* clientContext = m_clientContextMap.getClientContext(clientEquipmentID);

		TEST_PTR_RETURN_NULLPTR(clientContext);

		return clientContext->getSignalChangesQueue(tcpConnectionID);
	}

	void TuningServiceWorker::initialize()
	{
		runGrpcCfgLoaderThread();
		// runCfgLoaderThread();
	}

	void TuningServiceWorker::shutdown()
	{
		clearConfiguration();
		//stopCfgLoaderThread();
		stopGrpcCfgLoaderThread();

		if (m_tuningPacketLog != nullptr)
		{
			LOGGER_SHUTDOWN(m_tuningPacketLog);
		}
	}

	void TuningServiceWorker::clearConfiguration()
	{
		DEBUG_LOG_MSG(logger(), QString("Clear current configuration"));

		m_startStopMutex.lock();

		stopTcpTuningServerThread();
		stopSourcesListenerThreads();
		stopTuningSourceThreads();
		clearServiceMaps();

		m_startStopMutex.unlock();
	}

	void TuningServiceWorker::applyNewConfiguration(const TuningSources& newSources,
													std::shared_ptr<std::vector<char>> tuningSourcesFileData)
	{
		DEBUG_LOG_MSG(logger(), QString("Apply new configuration"));

		m_startStopMutex.lock();

		buildServiceMaps(newSources);
		runTuningSourceThreads();
		runSourcesListenerThreads();
		runTcpTuningServerThread(tuningSourcesFileData);

		m_startStopMutex.unlock();
	}

	void TuningServiceWorker::buildServiceMaps(const TuningSources& newSources)
	{
		m_tuningSources = newSources;
		fillControlledLans();
		m_clientContextMap.init(m_serviceSettings, m_tuningSources);
	}

	void TuningServiceWorker::clearServiceMaps()
	{
		m_controlledLans.clear();
		m_clientContextMap.clear();
		m_tuningSources.clear();
	}

	void TuningServiceWorker::fillControlledLans()
	{
		m_controlledLans.clear();

		for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
		{
			const TuningServiceSettings::ChannelSettings& ch = m_serviceSettings.channelSettings[channel];

			if (ch.enable == false)
			{
				continue;
			}

			for(auto& ts : ch.sources)
			{
				if (ts.isValid() == false)
				{
					Q_ASSERT(false);
					continue;
				}

				std::pair<QString, QString> srcLan = { ts.lmEquipmentID, ts.portEquipmentID };

				if (m_controlledLans.contains(srcLan) == true)
				{
					Q_ASSERT(false);
					continue;
				}

				m_controlledLans.insert(srcLan);
			}
		}
	}

	bool TuningServiceWorker::readConfiguration(const QByteArray& cfgXmlData)
	{
		bool result = true;

		result = softwareSettingsSet().readFromXml(cfgXmlData);

		if (result == true)
		{
			std::shared_ptr<const TuningServiceSettings> typedSettingsPtr =
					softwareSettingsSet().getSettingsProfile<TuningServiceSettings>(SettingsProfile::DEFAULT);

			if (typedSettingsPtr != nullptr)
			{
				m_serviceSettings = *typedSettingsPtr;
			}
			else
			{
				result = false;
			}
		}

		return result;
	}

	bool TuningServiceWorker::loadConfigurationFromFile(const QString& fileName)
	{
		QByteArray cfgXmlData;

		QFile file(fileName);

		if (file.open(QIODevice::ReadOnly) == false)
		{
			DEBUG_LOG_ERR(logger(), QString("Error open configuration file: %1").arg(fileName));
			return false;
		}

		cfgXmlData = file.readAll();

		bool result = true;

		result = readConfiguration(cfgXmlData);

		if  (result == true)
		{
			DEBUG_LOG_MSG(logger(), QString("Configuration is loaded from file: %1").arg(fileName));
		}
		else
		{
			DEBUG_LOG_ERR(logger(), QString("Loading configuration error from file: %1").arg(fileName));
		}

		return result;
	}

	bool TuningServiceWorker::readTuningSources(const QByteArray& fileData, const QString& profile, TuningSources* newSources)
	{
		TEST_PTR_RETURN_FALSE(newSources);

		newSources->clear();

		QVector<TuningSource> sources;
		OnlineLib::BuildInfo buildInfo;

		bool result = OnlineLib::DataSourcesXML<TuningSource>::readFromXml(fileData, &sources, &buildInfo);

		RETURN_IF_FALSE(result);

		for(const TuningSource& ts : sources)
		{
			if (ts.profile() == profile)
			{
				newSources->push_back(ts);
			}
		}

		newSources->buildMaps();

		return result;
	}

	void TuningServiceWorker::runTcpTuningServerThread(std::shared_ptr<std::vector<char>> tuningSourcesFileData)
	{
		Q_ASSERT(m_tcpTuningServerThread == nullptr);

		TcpTuningServer* tcpTuningSever = new TcpTuningServer(*this, m_tuningSources, tuningSourcesFileData, logger());

		m_tcpTuningServerThread = new TcpTuningServerThread(m_serviceSettings.clientRequestIP,
															m_serviceSettings.securityLevel,
															tcpTuningSever,
															logger());
		m_tcpTuningServerThread->start();
	}

	void TuningServiceWorker::stopTcpTuningServerThread()
	{
		if (m_tcpTuningServerThread != nullptr)
		{
			m_tcpTuningServerThread->quitAndWait();
			delete m_tcpTuningServerThread;
			m_tcpTuningServerThread = nullptr;

			DEBUG_LOG_MSG(logger(), QString("TcpTuningServerThread stoped"));
		}
	}

	void TuningServiceWorker::runTuningSourceThreads()
	{
		if (m_serviceSettings.singleLmControl == false)
		{
			// running all TuningSourceWorkers at once if SingleLmControl is disabled
			//
			runTuningSourceThread(false, QString(""));
		}
	}

	bool TuningServiceWorker::runTuningSourceThread(bool runSingleSource,
													const QString& singleSourceEquipmentID)
	{
		// if tuningSourceEquipmentID empty - run all sources workers
		// else - run specific source worker
		//
		assert(m_sourceThreads.size() == 0);

		bool result = false;

		for(const TuningSource& tuningSource : m_tuningSources)
		{
			if (runSingleSource == true && tuningSource.moduleEquipmentID() != singleSourceEquipmentID)
			{
				continue;
			}

			if (m_serviceSettings.isSourceExists(tuningSource.moduleEquipmentID()) == false)
			{
				continue;
			}

			if (tuningSource.hasTuningSignals() == false)
			{
				DEBUG_LOG_MSG(logger(),
							  QString("Tuning source %1 has no signals. Controlling thread wouldn't be run.").
							  arg(tuningSource.moduleEquipmentID()));
				continue;
			}

			// create TuningSourceWorkerThreads and fill m_sourceWorkerThreadMap
			//
			TuningSourceThreadShared sourceThread = createTuningSourceThread(tuningSource);

			TEST_PTR_CONTINUE(sourceThread);

			setSourceThreadInTuningClientContexts(sourceThread);

			result = true;
		}

		for(auto& p : m_sourceThreads)
		{
			TuningSourceThreadShared sourceThread = p.second;

			sourceThread->start();
			sourceThread->waitWhileHandlersInitialized();
		}

		return result;
	}

	TuningSourceThreadShared TuningServiceWorker::createTuningSourceThread(const TuningSource& source)
	{
		auto it = m_sourceThreads.find(source.moduleEquipmentID());

		if (it != m_sourceThreads.end())
		{
			Q_ASSERT(false);			// attempt to run duplicate TuningSourceThread
			return nullptr;
		}

		TuningSourceThreadShared sourceThread =
				std::make_shared<TuningSourceThread>(	*this,
														m_serviceSettings,
														source,
														sessionParams().softwareRunMode,
														logger(),
														m_tuningLog);

		m_sourceThreads.insert({source.moduleEquipmentID(), sourceThread});

		std::vector<quint32> IPs = source.lanControllersInfo().tuningIP32addresses();

		for(auto ip : IPs)
		{
			auto it2 = m_ip2sourceThread.find(ip);

			if (it2 == m_ip2sourceThread.end())
			{
				m_ip2sourceThread.insert({ip, sourceThread});
			}
			else
			{
				Q_ASSERT(false);				// duplicate IP
			}
		}

		return sourceThread;
	}

	void TuningServiceWorker::stopTuningSourceThreads()
	{
		for(auto& p : m_sourceThreads)
		{
			TuningSourceThreadShared sourceThread = p.second;

			TEST_PTR_CONTINUE(sourceThread)

			removeSourceThreadFromTuningClientContexts(sourceThread->sourceEquipmentID());

			sourceThread->quitAndWait();
		}

		m_sourceThreads.clear();
		m_ip2sourceThread.clear();
	}

	void TuningServiceWorker::runSourcesListenerThreads()
	{
		if (m_sourceThreads.size() == 0)
		{
			DEBUG_LOG_MSG(logger(), QString("Tuning sources workers is not running. Listener thread is not run also."));
			return;
		}

		// create and run TuningSocketListenerThread
		//
		Q_ASSERT(m_socketListenerThreads.size() == 0);

		for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
		{
			const TuningServiceSettings::ChannelSettings& ch = m_serviceSettings.channelSettings[channel];

			if (isSourceHandlerExistsForChannel(channel) == false)
			{
				DEBUG_LOG_MSG(logger(),
							  QString("No tuning sources found for channel %1. Therefore Listener of IP %2 will not be run.").
							  arg(channel + 1).arg(ch.tuningDataIP.addressPortStr()));
				continue;
			}

			CONTINUE_IF_FALSE(ch.enable);

			auto thread = new TuningSocketListenerThread(*this,
														 ch.tuningDataIP,
														 channel,
														 isSimulationMode(),
														 logger());
			m_socketListenerThreads.push_back(thread);

			thread->start();
		}
	}

	void TuningServiceWorker::stopSourcesListenerThreads()
	{
		// stop and delete TuningSocketListenerThread
		//
		for(auto thread : m_socketListenerThreads)
		{
			thread->quitAndWait();
			delete thread;
		}

		m_socketListenerThreads.clear();
	}

	void TuningServiceWorker::setSourceThreadInTuningClientContexts(TuningSourceThreadShared thread)
	{
		TEST_PTR_RETURN(thread);

		m_clientContextMap.setSourceThreadInTuningClientContexts(thread);
	}

	void TuningServiceWorker::removeSourceThreadFromTuningClientContexts(const QString& tuningSourceID)
	{
		m_clientContextMap.removeSourceThreadFromTuningClientContexts(tuningSourceID);
	}

	bool TuningServiceWorker::isSimulationMode() const
	{
		return sessionParams().softwareRunMode == E::SoftwareRunMode::Simulation;
	}

	bool TuningServiceWorker::isSourceHandlerExistsForChannel(int channel) const
	{
		for(auto& p : m_sourceThreads)
		{
			TEST_PTR_CONTINUE(p.second);

			if (p.second->isSourceHandlerExistsForChannel(channel) == true)
			{
				return true;
			}
		}

		return false;
	}

	void TuningServiceWorker::onConfigurationReady(const QByteArray configurationXmlData,
												   const BuildFileInfoArray buildFileInfoArray,
												   SessionParams sessionParams,
												   std::shared_ptr<const SoftwareSettings> curSettingsProfile)
	{
		setSessionParams(sessionParams);

		Q_UNUSED(configurationXmlData);

		const TuningServiceSettings* typedSettingsPtr = dynamic_cast<const TuningServiceSettings*>(curSettingsProfile.get());

		if (typedSettingsPtr == nullptr)
		{
			DEBUG_LOG_MSG(logger(), "Settings casting error!");
			return;
		}

		m_serviceSettings = *typedSettingsPtr;

		bool result = true;

		TuningSources newSources;
		std::shared_ptr<std::vector<char>> tuningSourcesFileData;

		for(OnlineLib::BuildFileInfo bfi : buildFileInfoArray)
		{
			QByteArray fileData;
			QString errStr;

			m_grpcCfgLoaderThread->getFileBlocked(bfi.pathFileName, &fileData, &errStr);

			if (errStr.isEmpty() == false)
			{
				DEBUG_LOG_ERR(logger(), errStr);
				result = false;
				continue;
			}

			result = true;

			if (bfi.ID == CfgFileId::TUNING_SOURCES)
			{
				result &= readTuningSources(fileData, sessionParams.currentSettingsProfile, &newSources);
				tuningSourcesFileData = std::make_shared<std::vector<char>>(fileData.begin(), fileData.end());
			}

			if (result == true)
			{
				DEBUG_LOG_MSG(logger(), QString("Read file %1 OK").arg(bfi.pathFileName));
			}
			else
			{
				DEBUG_LOG_ERR(logger(), QString("Read file %1 ERROR").arg(bfi.pathFileName));
				break;
			}
		}

		if (result == true)
		{
			DEBUG_LOG_MSG(logger(), QString("Configuration reading success"));

			clearConfiguration();

			m_buildInfo = m_grpcCfgLoaderThread->buildInfo();

			applyNewConfiguration(newSources, tuningSourcesFileData);
		}
	}
}
