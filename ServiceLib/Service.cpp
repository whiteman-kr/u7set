#ifndef SERVICE_LIB_DOMAIN
#error Don't include this file in the project! Link ServiceLib instead.
#endif

#include "Service.h"
#include "../UtilsLib/WUtils.h"
#include "../lib/ConstStrings.h"

// -------------------------------------------------------------------------------------
//
// ServiceWorker class implementation
//
// -------------------------------------------------------------------------------------

int ServiceWorker::m_instanceNo = 0;

ServiceWorker::ServiceWorker(const SoftwareInfo& softwareInfo,
							 const QString& serviceName,
							 int& argc,
							 char** argv,
							 CircularLoggerShared logger) :
	m_softwareInfo(softwareInfo),
	m_serviceName(serviceName),
	m_argc(argc),
	m_argv(const_cast<const char**>(argv)),
	m_logger(logger),
	m_serviceSettings(QSettings::SystemScope, Manufacturer::RADIY, serviceName, this),
	m_softwareSettingsSet(softwareInfo.softwareType())
{
	TEST_PTR_RETURN(argv);

	initThisInstanceNo();

	Q_ASSERT(m_thisInstanceNo == 1);

	copyCmdLineArgs(m_argc, m_argv);

	m_cmdLineParser = new CommandLineParser(m_cmdLineArgs);
}

ServiceWorker::ServiceWorker(const ServiceWorker* prevInstance) :
	m_softwareInfo(prevInstance->softwareInfo()),
	m_serviceName(prevInstance->serviceName()),
	m_logger(prevInstance->logger()),
	m_argc(prevInstance->argc()),
	m_argv(prevInstance->argv()),
	m_cmdLineArgs(prevInstance->cmdLineArgs()),
	m_serviceSettings(QSettings::SystemScope, Manufacturer::RADIY, prevInstance->serviceName(), this),
	m_softwareSettingsSet(prevInstance->softwareInfo().softwareType())
{
	initThisInstanceNo();

	Q_ASSERT(m_thisInstanceNo > 1);

	copyServiceSettings(prevInstance->serviceSettings());
}

ServiceWorker::~ServiceWorker()
{
	DELETE_IF_NOT_NULL(m_cmdLineParser);
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

bool ServiceWorker::initInstance1()
{
	if (m_thisInstanceNo > 1)
	{
		Q_ASSERT(false);
		return true;
	}

	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::HELP, "Print this help.");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::VERSION, "Display version of service.");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::EXEC_AS_APP, "Run service as a regular application.");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::INSTALL, "Install the service. Needs administrator rights.");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::UNINSTALL, "Uninstall the service. Needs administrator rights.");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::TERMINATE, "Terminate (stop) the service.");
	m_cmdLineParser->addValueCmdLineArg(CmdLineArg::INSTANCE, "ServiceInstanceID", "Set service instance ID.", "InstanceID");
	m_cmdLineParser->addSimpleNoWritableCmdLineArg(CmdLineArg::CLEAR, "Clear all service settings.");

	initCustomCmdLineArgs();

	m_cmdLineParser->parse();

	m_cmdLineParser->writeSettingsToRegistry(m_serviceSettings, m_logger);

	return true;
}

void ServiceWorker::setService(Service* service)
{
	m_service = service;
}

Service* ServiceWorker::service()
{
	assert(m_service != nullptr);
	return m_service;
}

void ServiceWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
	Q_UNUSED(serviceInfo);
}

bool ServiceWorker::clearSettings()
{
	m_serviceSettings.clear();

	m_serviceSettings.sync();

	return CommandLineParser::checkSettingWriteStatus(m_serviceSettings, "", nullptr);
}

QString ServiceWorker::getSettingValue(const QString& settingName)
{
	QVariant v = m_serviceSettings.value(settingName).toString();

	if (v.isNull() == true ||
		v.isValid() == false)
	{
		Q_ASSERT(false);
		return QString();
	}

	return v.toString();
}

bool ServiceWorker::getBoolSettingValue(const QString& settingName)
{
	QString valueStr = getSettingValue(settingName);

	bool ok = false;

	bool result = stringToBool(valueStr, &ok);

	if (ok == false)
	{
		Q_ASSERT(false);
		result = false;
	}

	return result;
}

QString ServiceWorker::getSoftwareInfoStr() const
{
	QString swInfo =
		QString("%1 %2.%3.%4 (%5) SHA: %6").
			arg(m_serviceName).
			arg(m_softwareInfo.majorVersion()).
			arg(m_softwareInfo.minorVersion()).
			arg(m_softwareInfo.commitNo()).
			arg(m_softwareInfo.buildBranch()).
			arg(m_softwareInfo.commitSHA());

	return swInfo;
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
	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	return m_cmdLineParser->addSimpleNoWritableCmdLineArg(cmdLineArgName, description);
}

bool ServiceWorker::addSimpleCmdLineArg(const QString& cmdLineArgName,
						 const QString& settingName,
						 const QString& description)
{
	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	return m_cmdLineParser->addSimpleCmdLineArg(cmdLineArgName, settingName, description);
}

bool ServiceWorker::addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
								  const QString& description,
								  const QString& paramExample)
{
	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	return m_cmdLineParser->addValueNoWritebleCmdLineArg(cmdLineArgName, description, paramExample);
}

bool ServiceWorker::addValueCmdLineArg(const QString& cmdLineArgName,
						const QString& settingName,
						const QString& description,
						const QString& paramExample)
{
	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	return m_cmdLineParser->addValueCmdLineArg(cmdLineArgName, settingName, description, paramExample);
}

bool ServiceWorker::cmdLineArgIsSet(const QString& cmdLineArgName) const
{
	TEST_PTR_RETURN_FALSE(m_cmdLineParser);

	return m_cmdLineParser->cmdLineArgIsSet(cmdLineArgName);
}

QString ServiceWorker::helpText() const
{
	TEST_PTR_RETURN_VALUE(m_cmdLineParser, QString());

	return m_cmdLineParser->helpText();
}

bool ServiceWorker::processCustomCmdLineArgs()
{
	return true;		// return TRUE to continue service running
						// return FALSE to exit service
}

void ServiceWorker::initThisInstanceNo()
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

void ServiceWorker::copyServiceSettings(const QSettings& st)
{
	Q_ASSERT(m_thisInstanceNo > 1);

	m_serviceSettings.clear();

	QStringList keys = st.allKeys();

	for(const QString& key : keys)
	{
		m_serviceSettings.setValue(key, st.value(key));
	}
}

void ServiceWorker::onThreadStarted()
{
	// loading common settings of services

	m_equipmentID = getSettingValue(SoftwareSetting::EQUIPMENT_ID);

	m_softwareInfo.setEquipmentID(m_equipmentID);		// !

	if (m_softwareInfo.softwareType() != E::SoftwareType::ConfigurationService)
	{
		m_cfgServiceIP1Str = getSettingValue(SoftwareSetting::CFG_SERVICE_IP1);

		m_cfgServiceIP1.setAddressPortStr(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

		m_cfgServiceIP2Str = getSettingValue(SoftwareSetting::CFG_SERVICE_IP2);

		m_cfgServiceIP2.setAddressPortStr(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	}

	//

	loadSettings();

	initialize();

	emit work();
}

void ServiceWorker::onThreadFinished()
{
	shutdown();
	emit stopped();
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

	startBaseRequestSocketThread();
}

void Service::stop()
{
	stopBaseRequestSocketThread();

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

void Service::onServiceWork()
{
	m_state = ServiceState::Work;
}

void Service::onServiceStopped()
{
	m_state = ServiceState::Stopped;
}

void Service::onBaseRequest(UdpRequest request)
{
	UdpRequest ack;

	ack.initAck(request);

	HostAddressPort ha(request.address().toIPv4Address(), request.port());

	switch(request.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			Network::ServiceInfo si;
			getServiceInfo(si);
			ack.writeData(si);
			break;
		}

		case RQID_SERVICE_START:
			LOG_MSG(m_logger, QString("Service START request from SCM (%1).").arg(ha.addressStr()));
			startServiceWorkerThread();
			break;

		case RQID_SERVICE_STOP:
			LOG_MSG(m_logger, QString("Service STOP request from SCM (%1).").arg(ha.addressStr()));
			stopServiceWorkerThread();
			break;

		case RQID_SERVICE_RESTART:
			LOG_MSG(m_logger, QString("Service RESTART request from SCM (%1).").arg(ha.addressStr()));
			stopServiceWorkerThread();
			startServiceWorkerThread();
			break;

		default:
			assert(false);
			ack.setErrorCode(RQERROR_UNKNOWN_REQUEST);
			break;
	}

	emit ackBaseRequest(ack);
}

void Service::startServiceWorkerThread()
{
	QMutexLocker locker(&m_mutex);

	if (m_serviceWorkerThread != nullptr)
	{
		assert(false);
		return;
	}

	if (m_state != ServiceState::Stopped)
	{
		assert(false);
		return;
	}

	m_serviceWorkerStartTime = QDateTime::currentMSecsSinceEpoch();

	m_state = ServiceState::Starts;

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

	m_state = ServiceState::Stops;

	m_serviceWorkerThread->quit();
	m_serviceWorkerThread->wait();

	delete m_serviceWorkerThread;

	m_serviceWorkerThread = nullptr;
	m_serviceWorker = nullptr;

	m_state = ServiceState::Stopped;
}

void Service::startBaseRequestSocketThread()
{
	E::SoftwareType swType = m_serviceWorkerFactory.softwareType();

	auto it = std::find_if(	servicesInfo.begin(),
							servicesInfo.end(),
							[swType](const ServiceInfo& si)
							{
								return si.softwareType == swType;
							});

	Q_ASSERT(it != servicesInfo.end());

	const ServiceInfo& sInfo = *it;

	UdpServerSocket* serverSocket = new UdpServerSocket(QHostAddress::AnyIPv4, sInfo.port, m_logger);

	connect(serverSocket, &UdpServerSocket::receiveRequest, this, &Service::onBaseRequest);
	connect(this, &Service::ackBaseRequest, serverSocket, &UdpServerSocket::sendAck);

	m_baseRequestSocketThread = new UdpSocketThread(serverSocket);

	m_baseRequestSocketThread->start();
}

void Service::stopBaseRequestSocketThread()
{
	m_baseRequestSocketThread->quitAndWait();

	delete m_baseRequestSocketThread;
}

void Service::getServiceInfo(Network::ServiceInfo& serviceInfo)
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

	if (m_serviceWorker != nullptr)
	{
		m_serviceWorker->getServiceSpecificInfo(serviceInfo);
	}

	if (m_state != ServiceState::Stopped)
	{
		serviceInfo.set_serviceruntime((QDateTime::currentMSecsSinceEpoch() - m_serviceWorkerStartTime) / 1000);
	}
	else
	{
		serviceInfo.set_serviceruntime(0);
	}
}

