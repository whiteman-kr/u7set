#ifndef SERVICE_LIB_DOMAIN
#error Do not include this file in the project! Link ServiceLib instead.
#endif

#include <QXmlStreamReader>

#include <ServiceLib/Service.h>
#include <ServiceLib/TcpSrvInfoServer.h>
#include "./qtservice/src/qtservice.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"
#include <CommonLib/ConstStrings.h>

// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

int ServiceWorker::m_instanceNo = 0;

ServiceWorker::ServiceWorker(const SoftwareInfo& softwareInfo,
							 const QString& serviceName,
							 int argc,
							 char** argv,
							 CircularLoggerShared logger,
							 const QString& workerName) :
	SimpleThreadWorker(workerName),
	m_softwareInfo(softwareInfo),
	m_serviceName(serviceName),
	m_argc(argc),
	m_argv(const_cast<const char**>(argv)),
	m_cmdLineParser(Manufacturer::RADIY, serviceName, argc, argv),
	m_logger(logger),
	m_softwareSettingsSet(softwareInfo.softwareType())
{
	TEST_PTR_RETURN(argv);

	setThisInstanceNo();

	Q_ASSERT(m_thisInstanceNo == 1);

	copyCmdLineArgs(m_argc, m_argv);
}

ServiceWorker::ServiceWorker(const ServiceWorker* prevInstance) :
	SimpleThreadWorker(prevInstance->workerName()),
	m_softwareInfo(prevInstance->softwareInfo()),
	m_serviceName(prevInstance->serviceName()),
	m_logger(prevInstance->logger()),
	m_argc(prevInstance->argc()),
	m_argv(prevInstance->argv()),
	m_cmdLineArgs(prevInstance->cmdLineArgs()),
	m_cmdLineParser(prevInstance->commandLineParser()),
	m_serviceRunMode(prevInstance->serviceRunMode()),
	m_softwareSettingsSet(prevInstance->softwareInfo().softwareType())
{
	setThisInstanceNo();

	Q_ASSERT(m_thisInstanceNo > 1);
}

ServiceWorker::~ServiceWorker()
{
}

QString ServiceWorker::appPath() const
{
	if (m_cmdLineArgs.size() < 1)
	{
		Q_ASSERT(false);
		return QString();
	}

	return m_cmdLineArgs[0];
}

QString ServiceWorker::cmdLine() const
{
	Q_ASSERT(m_cmdLineArgs.size() >= 1);

	return m_cmdLineArgs.join(" ");
}

QString ServiceWorker::serviceName() const
{
	return m_serviceName;
}

const SoftwareInfo& ServiceWorker::softwareInfo() const
{
	return m_softwareInfo;
}

E::SoftwareType ServiceWorker::softwareType() const
{
	return m_softwareInfo.softwareType();
}

void ServiceWorker::loadCommonServicesSettings()
{
	m_equipmentID = getSettingValue(SoftwareSetting::EQUIPMENT_ID);

	m_softwareInfo.setEquipmentID(m_equipmentID);		// !

	if (m_softwareInfo.softwareType() != E::SoftwareType::ConfigurationService)
	{
		m_cfgServiceIP1Str = getSettingValue(SoftwareSetting::CFG_SERVICE_IP1);

		m_cfgServiceIP1.setAddressPortStr(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

		m_cfgServiceIP2Str = getSettingValue(SoftwareSetting::CFG_SERVICE_IP2);

		m_cfgServiceIP2.setAddressPortStr(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	}
}

void ServiceWorker::setService(Service* service)
{
	m_service = service;
}

Service* ServiceWorker::service()
{
	Q_ASSERT(m_service != nullptr);
	return m_service;
}

void ServiceWorker::processGetServiceInfoRequest(const Network::GetServiceInfoRequest& rq)
{
	Q_UNUSED(rq);
}

void ServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo);
}

bool ServiceWorker::clearSettings()
{
	return m_cmdLineParser.clearSettings();
}

void ServiceWorker::runGrpcCfgLoaderThread()
{
	assert(m_grpcCfgLoaderThread == nullptr);			// once should be runned

	m_grpcCfgLoaderThread = std::make_unique<GrpcCfgLoaderThread>(softwareInfo(), 1,
																  cfgServiceIP1(), cfgServiceIP2(), logger());

	connect(m_grpcCfgLoaderThread.get(), &GrpcCfgLoaderThread::signal_configurationReady, this, &ServiceWorker::onConfigurationReady);

	m_grpcCfgLoaderThread->start();
}

void ServiceWorker::stopGrpcCfgLoaderThread()
{
	if (m_grpcCfgLoaderThread == nullptr)
	{
		return;
	}

	m_grpcCfgLoaderThread->quitAndWait();

	m_grpcCfgLoaderThread.reset();
}

void ServiceWorker::onConfigurationReady(const QByteArray configurationXmlData,
										const BuildFileInfoArray buildFileInfoArray,
										SessionParams sessionParams,
										std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
{
	Q_ASSERT(false);		// should be implemented in derived classes
}

QString ServiceWorker::getSettingValue(const QString& settingName) const
{
	return m_cmdLineParser.getSettingValue(settingName);
}

bool ServiceWorker::getBoolSettingValue(const QString& settingName) const
{
	QString valueStr = getSettingValue(settingName);

	bool ok = false;

	bool result = stringToBool(valueStr, &ok);

	if (ok == false)
	{
		qDebug() << "ServiceWorker::getBoolSettingValue: Cannot convert setting" << settingName << ", value " << valueStr << " to bool.";
		Q_ASSERT(false);
		result = false;
	}

	return result;
}

QStringList ServiceWorker::getSoftwareInfo() const
{
	const SoftwareInfo& si = m_softwareInfo;

	QStringList res;


	res << QString(" %1 v%2.%3.%4 (%5)").
			arg(m_serviceName).
			arg(si.majorVersion()).
			arg(si.minorVersion()).
			arg(si.patchVersion()).
			arg(si.branchName());
	res << QString();
#ifdef QT_DEBUG
	res << QString(" Build:          %1 Debug").arg(si.releaseType());
#else
	res << QString(" Build:          %1 Release").arg(si.releaseType());
#endif
	res << QString(" Branch name:    %1").arg(si.branchName());
	res << QString(" Commit SHA:     %1").arg(si.commitHash());
	res << QString(" Build date:     %1").arg(si.buildDate());
	res << QString(" Build username: %1").arg(si.buildUserName());
	res << QString(" Build hostname: %1").arg(si.buildHostname());
	res << QString(" Pipeline ID:    %1").arg(si.pipelineID());

	return res;
}

void ServiceWorker::setSessionParams(const SessionParams& sp)
{
	AUTO_LOCK(m_spMutex);

	m_sessionParams = sp;
}

SessionParams ServiceWorker::sessionParams() const
{
	AUTO_LOCK(m_spMutex);

	return m_sessionParams;
}

void ServiceWorker::setServiceRunMode(E::ServiceRunMode srm)
{
	m_serviceRunMode = srm;
}

E::ServiceRunMode ServiceWorker::serviceRunMode() const
{
	return m_serviceRunMode;
}

bool ServiceWorker::addSimpleNoWritableCmdLineArg(const QString& cmdLineArgName,
												  const QString& description)
{
	return m_cmdLineParser.addSimpleNoWritableCmdLineArg(cmdLineArgName, description);
}

bool ServiceWorker::addSimpleCmdLineArg(const QString& cmdLineArgName,
						 const QString& settingName,
						 const QString& description)
{
	return m_cmdLineParser.addSimpleCmdLineArg(cmdLineArgName, settingName, description);
}

bool ServiceWorker::addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
								  const QString& description,
								  const QString& paramExample)
{
	return m_cmdLineParser.addValueNoWritebleCmdLineArg(cmdLineArgName, description, paramExample);
}

bool ServiceWorker::addValueCmdLineArg(const QString& cmdLineArgName,
						const QString& settingName,
						const QString& description,
						const QString& paramExample)
{
	return m_cmdLineParser.addValueCmdLineArg(cmdLineArgName, settingName, description, paramExample);
}

bool ServiceWorker::addBoolCmdLineArg(const QString& cmdLineArgName,
						const QString& settingName,
						const QString& description)
{
	return m_cmdLineParser.addBoolCmdLineArg(cmdLineArgName, settingName, description);
}

bool ServiceWorker::cmdLineArgIsSet(const QString& cmdLineArgName) const
{
	return m_cmdLineParser.cmdLineArgIsSet(cmdLineArgName);
}

QString ServiceWorker::helpText() const
{
	return m_cmdLineParser.helpText();
}

void ServiceWorker::setBuildInfo(const OnlineLib::BuildInfo& buildInfo)
{
	m_buildInfo = buildInfo;
}

OnlineLib::BuildInfo ServiceWorker::buildInfo() const
{
	return m_buildInfo;
}

void ServiceWorker::clearBuildInfo()
{
	m_buildInfo.clear();
}

bool ServiceWorker::readBuildInfo(const QByteArray& cfgXmlData)
{
	XmlReadHelper xmlReader(cfgXmlData);

	return m_buildInfo.readFromXml(xmlReader);
}

const QString ServiceWorker::cmdLineArg(int index) const
{
	if (index < 0 || index >= m_cmdLineArgs.size())
	{
		return Separator::EMPTY_STR;
	}

	return m_cmdLineArgs[index];
}

bool ServiceWorker::processServiceSpecificCmdLineArgs()
{
	return true;		// return TRUE to continue service running
						// return FALSE to exit service
}

void ServiceWorker::setThisInstanceNo()
{
	m_instanceNo++;
	m_thisInstanceNo = m_instanceNo;
}

void ServiceWorker::copyCmdLineArgs(int argc, const char** argv)
{
	Q_ASSERT(argc >= 1);
	TEST_PTR_RETURN(argv);

	Q_ASSERT(m_thisInstanceNo == 1);

	m_cmdLineArgs.clear();

	for(int i = 0; i < argc; i++)
	{
		TEST_PTR_CONTINUE(argv[i]);

		m_cmdLineArgs.append(QString(argv[i]).trimmed());
	}
}

bool ServiceWorker::initInstance1()
{
	if (m_thisInstanceNo > 1)
	{
		Q_ASSERT(false);
		return true;
	}

	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::HELP, "Print this help.");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::VERSION, "Display version of service.");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::EXEC_AS_APP, "Run service as a regular application.");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::INSTALL, "Install the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::UNINSTALL, "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::TERMINATE, "Terminate (stop) the service.");
	m_cmdLineParser.addValueCmdLineArg(CmdLineArg::INSTANCE, "ServiceInstanceID", "Set service instance ID.", "InstanceID");
	m_cmdLineParser.addSimpleNoWritableCmdLineArg(CmdLineArg::CLEAR, "Clear all service settings.");

	initServiceSpecificCmdLineArgs();

	m_cmdLineParser.readAndApplySettingsFromRegistry();

	m_cmdLineParser.parseAndApplyCmdLineArgs();

	m_cmdLineParser.writeSettingsToRegistry(m_logger);

	return true;
}

void ServiceWorker::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("%1::onThreadStarted(), instanceNo = %2, RunMode = %3").
								arg(metaObject()->className()).
								arg(m_thisInstanceNo).
								arg(E::valueToString<E::ServiceRunMode>(m_serviceRunMode)));

	loadCommonServicesSettings();
	loadServiceSpecificSettings();

	initialize();

	emit work();
}

void ServiceWorker::onThreadFinished()
{
	shutdown();

	emit stopped();

	DEBUG_LOG_MSG(m_logger, QString("%1::onThreadFinished(), instanceNo = %2").
								arg(metaObject()->className()).
								arg(m_thisInstanceNo));
}

// -------------------------------------------------------------------------------------
//
// Service class implementation
//
// -------------------------------------------------------------------------------------

Service::Service(ServiceWorker& serviceWorker, std::shared_ptr<CircularLogger> logger):
	m_logger(logger),
	m_serviceStartTime(QDateTime::currentMSecsSinceEpoch()),
	m_serviceWorkerFactory(serviceWorker),
	m_timer500ms(this)
{
}

Service::~Service()
{
}

void Service::start()
{
	startServiceWorkerThread();

	QThread::msleep(100);

	startUdpSrvInfoThread();
	startTcpSrvInfoThread();
}

void Service::stop()
{
	stopTcpSrvInfoThread();
	stopUdpSrvInfoThread();

	stopServiceWorkerThread();
}

QString Service::getServiceInstanceName(const QString& serviceName, const QString& instanceID)
{
	if (instanceID.isEmpty() == true)
	{
		return serviceName;
	}

	return QString("%1 (%2)").arg(serviceName).arg(instanceID);
}

QString Service::getServiceInstanceName(const QString& serviceName, int argc, char* argv[])
{
	return getServiceInstanceName(serviceName, getServiceInstanceID(argc, argv));
}

void Service::processGetServiceInfoRequest(const Network::GetServiceInfoRequest& rq)
{
	ServiceWorker* serviceWorker = m_serviceWorker;

	if (serviceWorker == nullptr)
	{
		serviceWorker = &m_serviceWorkerFactory;
	}

	serviceWorker->processGetServiceInfoRequest(rq);
}

void Service::getServiceInfo(Network::ServiceInfo& serviceInfo, bool shortInfo)
{
	ServiceWorker* serviceWorker = m_serviceWorker;

	if (serviceWorker == nullptr)
	{
		serviceWorker = &m_serviceWorkerFactory;
	}

	QMutexLocker locker(&m_mutex);

	Network::SoftwareInfo* si = new Network::SoftwareInfo();

	serviceWorker->softwareInfo().serializeTo(si);

	serviceInfo.set_allocated_softwareinfo(si);
	serviceInfo.set_uptime((QDateTime::currentMSecsSinceEpoch() - m_serviceStartTime) / 1000);

	serviceInfo.set_servicestate(TO_INT(m_state));

	Network::SessionParams* sp = new Network::SessionParams();

	serviceWorker->sessionParams().saveTo(sp);

	serviceInfo.set_allocated_sessionparams(sp);

	if (shortInfo == false)
	{
		if (m_serviceWorker != nullptr)
		{
			m_serviceWorker->buildInfo().saveToProto(serviceInfo.mutable_buildinfo());
			m_serviceWorker->getServiceSpecificInfo(serviceInfo);
		}
	}

	if (m_state != E::ServiceState::Stopped)
	{
		serviceInfo.set_serviceruntime((QDateTime::currentMSecsSinceEpoch() - m_serviceWorkerStartTime) / 1000);
	}
	else
	{
		serviceInfo.set_serviceruntime(0);
	}
}

std::shared_ptr<CircularLogger> Service::logger()
{
	return m_logger;
}

void Service::startServiceWorkerThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceWorkerThread != nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_state != E::ServiceState::Stopped)
	{
		Q_ASSERT(false);
		return;
	}

	m_serviceWorkerStartTime = QDateTime::currentMSecsSinceEpoch();

	m_state = E::ServiceState::Starts;

	m_serviceWorker = m_serviceWorkerFactory.createInstance();

	m_serviceWorker->setService(this);

	connect(m_serviceWorker, &ServiceWorker::work, this, &Service::onServiceWork);
	connect(m_serviceWorker, &ServiceWorker::stopped, this, &Service::onServiceStopped);

	m_serviceWorkerThread = new SimpleThread(m_serviceWorker);
	m_serviceWorkerThread->start();
}

void Service::stopServiceWorkerThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceWorkerThread == nullptr)
	{
		return;
	}

	m_state = E::ServiceState::Stops;

	m_serviceWorkerThread->quitAndWait();

	delete m_serviceWorkerThread;

	m_serviceWorkerThread = nullptr;
	m_serviceWorker = nullptr;

	m_state = E::ServiceState::Stopped;
}

const ServiceInfo* Service::getServiceInfo()
{
	E::SoftwareType swType = m_serviceWorkerFactory.softwareType();

	auto it = std::find_if(	servicesInfo.begin(),
						   servicesInfo.end(),
						   [swType](const ServiceInfo& si)
						   {
							   return si.softwareType == swType;
						   });

	if (it == servicesInfo.end())
	{
		Q_ASSERT(false);
		return nullptr;
	}

	return &(*it);
}

void Service::onServiceWork()
{
	m_state = E::ServiceState::Work;
}

void Service::onServiceStopped()
{
	m_state = E::ServiceState::Stopped;
}

void Service::onGetSrvShortInfoRequest(UdpRequest request)
{
	UdpRequest ack;

	ack.initAck(request);

	quint32 rqID = request.ID();

	switch(rqID)
	{
	case RQID_SERVICE_GET_SHORT_INFO:
		{
			Network::ServiceInfo si;
			getServiceInfo(si, true);
			ack.writeData(si);
		}
		break;

	default:
		Q_ASSERT(false);
		ack.setErrorCode(RQERROR_UNKNOWN_REQUEST);
		break;
	}

	emit ackBaseRequest(ack);
}

void Service::startUdpSrvInfoThread()
{
	const ServiceInfo* sInfo = getServiceInfo();

	TEST_PTR_RETURN(sInfo);

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::AnyIPv4, sInfo->udpPort, m_logger);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &Service::onGetSrvShortInfoRequest);
	connect(this, &Service::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	m_udpSrvInfoThread = new UdpSocketThread(serverSocket);

	m_udpSrvInfoThread->start();
}

void Service::stopUdpSrvInfoThread()
{
	if (m_udpSrvInfoThread != nullptr)
	{
		m_udpSrvInfoThread->quitAndWait();
		delete m_udpSrvInfoThread;
		m_udpSrvInfoThread = nullptr;
	}
}

void Service::startTcpSrvInfoThread()
{
	const ServiceInfo* sInfo = Service::getServiceInfo();

	TEST_PTR_RETURN(sInfo);

	HostAddressPort listenAddr;

	listenAddr.setSpecAddress(QHostAddress::AnyIPv4);
	listenAddr.setPort(sInfo->tcpPort);

	m_tcpSrvInfoThread = new Tcp::ListenerThread(listenAddr,
												 E::SecurityLevel::Basic,
												 new TcpSrvInfoServer(m_serviceWorkerFactory.softwareInfo(), "TcpSrvInfoServer", *this),
												 m_logger, "TcpSrvInfoServerListener");
	m_tcpSrvInfoThread->start();
}

void Service::stopTcpSrvInfoThread()
{
	if (m_tcpSrvInfoThread != nullptr)
	{
		m_tcpSrvInfoThread->quitAndWait();
		delete m_tcpSrvInfoThread;
		m_tcpSrvInfoThread = nullptr;
	}
}



