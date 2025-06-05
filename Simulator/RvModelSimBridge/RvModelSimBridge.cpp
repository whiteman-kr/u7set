#include "../UtilsLib/WUtils.h"
#include "RvModelSimBridge.h"
#include "ModelLinkThread.h"


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
	ServiceWorker(softwareInfo, serviceName, argc, argv, logger),
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
	clear();
}

ServiceWorker* ModelSimBridgeWorker::createInstance() const
{
	ModelSimBridgeWorker* newInstance = new ModelSimBridgeWorker(this);
	return newInstance;
}

void ModelSimBridgeWorker::getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const
{
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
					   QString("IP-address that receives packets from the model (default - 127.0.0.1)."),
					   "127.0.0.1");

	addValueCmdLineArg("modelPort",
					   "modelPort",
					   QString("IP-address that receives packets from the model (default - 9999)."),
					   "9999");
}

void ModelSimBridgeWorker::loadServiceSpecificSettings()
{
	m_modelIP = getSettingValue("modelIP");
	if (m_modelIP.isEmpty() == true)
	{
		m_modelIP = "127.0.0.1";
	}
	bool ok = false;
	m_modelPort = getSettingValue("modelPort").toInt(&ok);
	if (ok == false)
	{
		m_modelPort = 9999;
	}

	DEBUG_LOG_MSG(logger(), "");
	DEBUG_LOG_MSG(logger(), QString(tr("Service settings:")));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::EQUIPMENT_ID).arg(equipmentID()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP1).arg(cfgServiceIP1().addressPortStrIfSet()));
	DEBUG_LOG_MSG(logger(), QString(tr("%1 = %2")).arg(SoftwareSetting::CFG_SERVICE_IP2).arg(cfgServiceIP2().addressPortStrIfSet()));

	DEBUG_LOG_MSG(logger(), QString(tr("modelIP = %1")).arg(m_modelIP));
	DEBUG_LOG_MSG(logger(), QString(tr("modelPort = %1")).arg(m_modelPort));
	DEBUG_LOG_MSG(logger(), "");
}

void ModelSimBridgeWorker::clear()
{
	//m_tuningSources.clear();
}

void ModelSimBridgeWorker::initialize()
{
	runUdpModelLinkThread();
}

void ModelSimBridgeWorker::shutdown()
{
	//stopUdpModelLinkThread();
}

void ModelSimBridgeWorker::runUdpModelLinkThread()
{
	Q_ASSERT(m_udpModelLinkThread == nullptr);

	HostAddressPort addr{m_modelIP, m_modelPort};

	UdpModelLink* udpModelLink = new UdpModelLink(addr, logger());

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
	/*
	if (m_sourceThreads.size() == 0)
	{
		DEBUG_LOG_MSG(logger(), QString("Tuning sources workers is not running. Listener thread is not run also."));
		return;
	}

	// create and run TuningSocketListenerThread
	//
	Q_ASSERT(m_socketListenerThreads.size() == 0);

	for (int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
	{
		const TuningServiceSettings::ChannelSettings& ch = m_serviceSettings.channelSettings[channel];

		if (isSourceHandlerExistsForChannel(channel) == false)
		{
			DEBUG_LOG_MSG(logger(),
						  QString("No tuning sources found for channel %1. Therefore Listener of IP %2 will not be run.")
							  .arg(channel + 1)
							  .arg(ch.tuningDataIP.addressPortStr()));
			continue;
		}

		CONTINUE_IF_FALSE(ch.enable);

		auto thread = new TuningSocketListenerThread(*this, ch.tuningDataIP, channel, isSimulationMode(), logger());
		m_socketListenerThreads.push_back(thread);

		thread->start();
	}*/
}

void ModelSimBridgeWorker::stopSimulatorLinkThread()
{
	/* stop and delete TuningSocketListenerThread
	//
	for (auto thread : m_socketListenerThreads)
	{
		thread->quitAndWait();
		delete thread;
	}

	m_socketListenerThreads.clear();*/
}
