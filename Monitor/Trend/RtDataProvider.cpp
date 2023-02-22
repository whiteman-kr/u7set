#include "RtDataProvider.h"

RtConnection::RtConnection(const SoftwareInfo& softwareInfo,
						   MonitorSettings::AppDataService server,
						   const ISignalDataServer& signalDataServer,
						   ILogFile* logFile) :
	QObject(),
	m_logFile(logFile),
	m_server(server)
{
	Q_ASSERT(m_logFile);
	m_logFile->writeMessage(QString("TrendRtConn::TrendRtConn, server %1 (%2), address %3.")
							.arg(m_server.equipmentId, m_server.shortenId, m_server.realtimeAddress.toString()));

	// --
	//
	m_rtTcpClient = new RtTrendTcpClient(softwareInfo, m_server.realtimeAddress, server.equipmentId, signalDataServer, m_logFile);

	m_rtTcpClientThread = std::make_unique<SimpleThread>(m_rtTcpClient);
	m_rtTcpClientThread->start();

	connect(m_rtTcpClient, &RtTrendTcpClient::dataReady, this, &RtConnection::dataReady, Qt::QueuedConnection);
	connect(m_rtTcpClient, &RtTrendTcpClient::requestError, this, &RtConnection::requestError, Qt::QueuedConnection);
	connect(m_rtTcpClient, &RtTrendTcpClient::connectionLost, this, &RtConnection::connectionLost, Qt::QueuedConnection);

	return;
}

RtConnection::~RtConnection()
{
	m_logFile->writeMessage(QString("RtTrend %1 dtor.").arg(m_server.equipmentId));

	m_rtTcpClientThread->quitAndWait(10000);

	return;
}

bool RtConnection::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
{
	return m_rtTcpClient->setData(samplePeriod, trendSignals);
}

void RtConnection::setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod)
{
	m_rtTcpClient->setSamplePeriod(samplePeriod);
}

const MonitorSettings::AppDataService& RtConnection::server() const
{
	return m_server;
}

RtTrendTcpClient::Stat RtConnection::statistics() const
{
	return m_rtTcpClient->stat();
}

//
//
//		RtDataProvider
//
//
RtDataProvider::RtDataProvider(const MonitorConfigController& configController,
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

RtDataProvider::~RtDataProvider()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return;
}

void RtDataProvider::clear()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	m_createdConnectionsServers.clear();
	m_connections.clear();

	return;
}

void RtDataProvider::createConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	clear();

	m_createdConnectionsServers = m_configController.configuration().appDataRealTimeServices;

	const SoftwareInfo& softwareInfo = m_configController.softwareInfo();

	for (const MonitorSettings::AppDataService& server : m_createdConnectionsServers)
	{
		RtConnection& conn = m_connections.emplace_back(softwareInfo, server, m_signalDataServer, m_logFile);

		connect(&conn, &RtConnection::dataReady, this, &RtDataProvider::dataReady);
		connect(&conn, &RtConnection::requestError, this, &RtDataProvider::requestError);
		connect(&conn, &RtConnection::connectionLost, this, &RtDataProvider::connectionLost);
	}

	return;
}

void RtDataProvider::updateConnections()
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

bool RtDataProvider::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	for (auto& conn : m_connections)
	{
		// Add all signals to conneccvtions, because posiible the situaltion when signal were not downloaded yet from the server, and then
		// m_signalDataServer.dataServiceHasSignal cannot provide valid data, so, we set all signals and
		// in connection each time use only right signals
		//
		conn.setData(samplePeriod, trendSignals);
	}

	return true;
}

void RtDataProvider::setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod)
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	for (auto& conn : m_connections)
	{
		conn.setSamplePeriod(samplePeriod);
	}

	return;
}

size_t RtDataProvider::size() const
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return m_connections.size();
}

RtTrendTcpClient::Stat RtDataProvider::statistics() const
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

