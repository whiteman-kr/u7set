#include "../UtilsLib/WUtils.h"
#include "RvModelSimBridge.h"
#include "ModelLinkThread.h"
#include "SimLinkThread.h"


// -------------------------------------------------------------------------------------
//
// ModelSimBridgeWorker class implementation
//
// -------------------------------------------------------------------------------------

ModelSimBridgeWorker::ModelSimBridgeWorker(const SoftwareInfo& softwareInfo,
										 const QString& serviceName,
										 int argc,
										 char** argv,
										 CircularLoggerShared logger,
										 CircularLoggerShared simLog) :
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger, "ModelSimBridgeWorker"),
	m_simLog(simLog)
{
}

ModelSimBridgeWorker::ModelSimBridgeWorker(const ModelSimBridgeWorker* worker) :
	ServiceWorker(worker),
	m_simLog(worker->simLog())
{
}

ModelSimBridgeWorker::~ModelSimBridgeWorker()
{
}

ServiceWorker* ModelSimBridgeWorker::createInstance() const
{
	ModelSimBridgeWorker* newInstance = new ModelSimBridgeWorker(this);
	return newInstance;
}

void ModelSimBridgeWorker::getServiceSpecificInfo(Network::ServiceInfo& /*serviceInfo*/) const
{
//	Q_UNUSED(serviceInfo);

	/*QMutexLocker l(&m_startStopMutex);

	QString xmlString = SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType::TuningService, m_serviceSettings);

	serviceInfo.set_settingsxml(xmlString.toStdString());

	if (m_tcpTuningServerThread != nullptr)
	{
		m_tcpTuningServerThread->getClientsList(&serviceInfo);
	}

	m_tuningSources.getTuningSourcesInfo(&serviceInfo);

	int srcCount = serviceInfo.tuningsourcesinfostate_size();

	for (int i = 0; i < srcCount; i++)
	{
		const Network::DataSourceInfo& dsi = serviceInfo.tuningsourcesinfostate(i).info();

		TuningSourceThreadShared thread = getValueOrNullptr(m_sourceThreads, QString::fromStdString(dsi.moduleequipmentid()));

		TEST_PTR_CONTINUE(thread);

		thread->getSourceState(serviceInfo.mutable_tuningsourcesinfostate(i)->mutable_state());
	}*/
}

CircularLoggerShared ModelSimBridgeWorker::simLog() const
{
	return m_simLog;
}

void ModelSimBridgeWorker::initServiceSpecificCmdLineArgs()
{
	addValueCmdLineArg(CmdLineArg::ID, SoftwareSetting::EQUIPMENT_ID, "Service EquipmentID.", "EQUIPMENT_ID");
	
	addValueCmdLineArg(
		CmdLineArg::CFG_IP1,
		SoftwareSetting::CFG_SERVICE_IP1,
		QString("IP-address of first Configuration Service (default port - %1).").arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
		"ip[:port]");
	
	addValueCmdLineArg(
		CmdLineArg::CFG_IP2,
		SoftwareSetting::CFG_SERVICE_IP2,
		QString("IP-address of second Configuration Service (default port - %1).").arg(PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST),
		"ip[:port]");

	addValueCmdLineArg("modelIP",
					   "modelIP",
					   QString("IP-address that receives packets from the model (default - any address, %1).").arg(m_modelIP),
					   m_modelIP);

	addValueCmdLineArg("requestPort",
					   "requestPort",
					   QString("Port that receives packets from the model (default - %1).").arg(m_modelRequestPort),
					   QString::number(m_modelRequestPort));

	addValueCmdLineArg("replyPort",
					   "replyPort",
					   QString("Port to which replies are sent to the model (default - %1).").arg(m_modelReplyPort),
					   QString::number(m_modelReplyPort));

	addValueCmdLineArg("simIP", "simIP", QString("IP-address of the Simulator (default - %1).").arg(m_simIP), m_simIP);

	addValueCmdLineArg("simPort",
					   "simPort",
					   QString("Port that is used to connect to the Simulator (default - %1).").arg(m_simPort),
					   QString::number(m_simPort));
}

void ModelSimBridgeWorker::loadServiceSpecificSettings()
{
	bool ok = false;

	QString strValue = getSettingValue("modelIP");
	if (strValue.isEmpty() == false)
	{
		m_modelIP = strValue;
	}

	int intValue = getSettingValue("requestPort").toInt(&ok);
	if (ok == true)
	{
		m_modelRequestPort = intValue;
	}

	intValue = getSettingValue("replyPort").toInt(&ok);
	if (ok == true)
	{
		m_modelReplyPort = intValue;
	}

	strValue = getSettingValue("simIP");
	if (strValue.isEmpty() == false)
	{
		m_simIP = strValue;
	}

	intValue = getSettingValue("simPort").toInt(&ok);
	if (ok == true)
	{
		m_simPort = intValue;
	}

	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));

	DEBUG_LOG_MSG(logger(), QString(tr("modelIP = %1")).arg(m_modelIP));
	DEBUG_LOG_MSG(logger(), QString(tr("requestPort = %1")).arg(m_modelRequestPort));
	DEBUG_LOG_MSG(logger(), QString(tr("replyPort = %1")).arg(m_modelReplyPort));
	DEBUG_LOG_MSG(logger(), QString(tr("simIP = %1")).arg(m_simIP));
	DEBUG_LOG_MSG(logger(), QString(tr("simPort = %1")).arg(m_simPort));
	DEBUG_LOG_MSG(logger(), "");
}

void ModelSimBridgeWorker::initialize()
{
	runUdpModelLinkThread();
	runSimulatorLinkThread();

	connect(m_udpModelLinkThread,
			&UdpModelLinkThread::requestsArrived,
			[this]()
			{
				m_simLinkThread->pushRequests(m_udpModelLinkThread->popAllRequests());
			});

	connect(m_simLinkThread,
			&SimLinkThread::repliesReady,
			[this]()
			{
				m_udpModelLinkThread->pushReplies(m_simLinkThread->popAllReplies());
			});
}

void ModelSimBridgeWorker::shutdown()
{
	stopUdpModelLinkThread();
	stopSimulatorLinkThread();
}

void ModelSimBridgeWorker::runUdpModelLinkThread()
{
	Q_ASSERT(m_udpModelLinkThread == nullptr);

	HostAddressPort listenAddr{m_modelIP, m_modelRequestPort};

	UdpModelLink* udpModelLink = new UdpModelLink(listenAddr, m_modelReplyPort, logger(), m_simLog);

	m_udpModelLinkThread = new UdpModelLinkThread(udpModelLink);
	m_udpModelLinkThread->start();
}

void ModelSimBridgeWorker::stopUdpModelLinkThread()
{
	if (m_udpModelLinkThread != nullptr)
	{
		m_udpModelLinkThread->quitAndWait();
		delete m_udpModelLinkThread;
		m_udpModelLinkThread = nullptr;

		DEBUG_LOG_MSG(logger(), QString("UdpModelLinkThread stoped"));
	}
}

void ModelSimBridgeWorker::runSimulatorLinkThread()
{
	Q_ASSERT(m_simLinkThread == nullptr);

	HostAddressPort addr{m_simIP, m_simPort};

	SimLink* simLink = new SimLink(addr, logger(), m_simLog);

	m_simLinkThread = new SimLinkThread(simLink);
	m_simLinkThread->start();
}

void ModelSimBridgeWorker::stopSimulatorLinkThread()
{
	if (m_simLinkThread != nullptr)
	{
		m_simLinkThread->quitAndWait();
		delete m_simLinkThread;
		m_simLinkThread = nullptr;

		DEBUG_LOG_MSG(logger(), QString("SimLinkThread stoped"));
	}
}
