#include "MonitorTrendRealtimeConnections.h"

MonitorTrendRealtimeConnection::MonitorTrendRealtimeConnection(const SoftwareInfo& softwareInfo,
															   MonitorSettings::AppDataService server,
															   ILogFile* logFile) :
	QObject(),
	m_logFile(logFile),
	m_server(server)
{
	Q_ASSERT(m_logFile);
	m_logFile->writeMessage(QString("RtTrend %1 ctor.").arg(m_server.equipmentId));

	// --
	//
	m_rtTcpClient = new RtTrendTcpClient(softwareInfo, server.address, m_logFile);

	m_rtTcpClientThread = std::make_unique<SimpleThread>(m_rtTcpClient);
	m_rtTcpClientThread->start();

	connect(m_rtTcpClient, &RtTrendTcpClient::dataReady, this, &MonitorTrendRealtimeConnection::dataReady, Qt::QueuedConnection);
	connect(m_rtTcpClient, &RtTrendTcpClient::requestError, this, &MonitorTrendRealtimeConnection::requestError, Qt::QueuedConnection);
	connect(m_rtTcpClient, &RtTrendTcpClient::connectionLost, this, &MonitorTrendRealtimeConnection::connectionLost, Qt::QueuedConnection);

	return;
}

MonitorTrendRealtimeConnection::~MonitorTrendRealtimeConnection()
{
	m_logFile->writeMessage(QString("RtTrend %1 dtor.").arg(m_server.equipmentId));

	m_rtTcpClientThread->quitAndWait(10000);

	return;
}

bool MonitorTrendRealtimeConnection::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
{
	return m_rtTcpClient->setData(samplePeriod, trendSignals);
}

const MonitorSettings::AppDataService& MonitorTrendRealtimeConnection::server() const
{
	return m_server;
}

RtTrendTcpClient::Stat MonitorTrendRealtimeConnection::statistics() const
{
	return m_rtTcpClient->stat();
}

//
//
//		MonitorTrendRealtimeConnections
//
//
MonitorTrendRealtimeConnections::MonitorTrendRealtimeConnections(const MonitorConfigController& configController,
																 const ISignalDataServer& signalDataServer,
																 ILogFile* logFile) :
	QObject(),
	m_configController(configController),
	m_signalDataServer(signalDataServer),
	m_logFile(logFile)
{
	Q_ASSERT(m_logFile);
	return;
}

MonitorTrendRealtimeConnections::~MonitorTrendRealtimeConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return;
}

void MonitorTrendRealtimeConnections::clear()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	m_createdConnectionsServers.clear();
	m_connections.clear();

	return;
}

void MonitorTrendRealtimeConnections::createConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	clear();

	m_createdConnectionsServers = m_configController.configuration().appDataRealTimeServices;

	const SoftwareInfo& softwareInfo = m_configController.softwareInfo();

	for (const MonitorSettings::AppDataService& server : m_createdConnectionsServers)
	{
		MonitorTrendRealtimeConnection& conn = m_connections.emplace_back(softwareInfo, server,m_logFile);

		connect(&conn, &MonitorTrendRealtimeConnection::dataReady, this, &MonitorTrendRealtimeConnections::dataReady);
		connect(&conn, &MonitorTrendRealtimeConnection::requestError, this, &MonitorTrendRealtimeConnections::requestError);
		connect(&conn, &MonitorTrendRealtimeConnection::connectionLost, this, &MonitorTrendRealtimeConnections::connectionLost);
	}

	return;
}

void MonitorTrendRealtimeConnections::updateConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	// New configuration arrived, server configuration could be changed,
	// if so, restart comminications threads.
	//
	m_logFile->writeMessage("RtTrends: New configuration arrived.");

	auto rts = m_configController.configuration().appDataRealTimeServices;

	if (std::ranges::equal(m_createdConnectionsServers, rts) == false)
	{
		clear();
		createConnections();
	}

	return;
}

size_t MonitorTrendRealtimeConnections::size() const
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return m_connections.size();
}

bool MonitorTrendRealtimeConnections::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	for (auto& conn : m_connections)
	{
		QStringList thisServerSignals;

		for (const QString& signalId : trendSignals)
		{
			if 	(m_signalDataServer.dataServiceHasSignal(conn.server().equipmentId, signalId) == true)
			{
				thisServerSignals.push_back(signalId);
			}
		}

		conn.setData(samplePeriod, thisServerSignals);
	}

	return true;
}

RtTrendTcpClient::Stat MonitorTrendRealtimeConnections::statistics() const
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	RtTrendTcpClient::Stat result{};

	for (const auto& conn : m_connections)
	{
		RtTrendTcpClient::Stat cs = conn.statistics();

		if (cs.text.isEmpty() == false)
		{
			if (result.text.isEmpty() == false)
			{
				result.text += " | ";
			}

			result.text += cs.text;
		}

		result.requestQueueSize += cs.requestQueueSize;
		result.requestCount += cs.requestCount;
		result.replyCount += cs.replyCount;
		result.isConnected += cs.isConnected;
	}

	return result;
}

