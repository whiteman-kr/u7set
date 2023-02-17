#include "MonitorTrendArchiveConnections.h"


MonitorTrendArchiveConnection::MonitorTrendArchiveConnection(const SoftwareInfo& softwareInfo,
															 MonitorSettings::ArchiveService server,
															 ILogFile* logFile) :
	QObject(),
	m_logFile(logFile),
	m_archiveServer(server.equipmentId, server.shortenId, server.appDataServiceId)
{
	Q_ASSERT(m_logFile);
	m_logFile->writeMessage(QString("TrendArchive::TrendArchive, server %1 (%2), address %3.")
							.arg(server.equipmentId, server.shortenId, server.address.toString()));

	// --
	//
	m_archiveTcpClient = new ArchiveTrendTcpClient{softwareInfo, server, m_logFile};

	m_archiveTcpClientThread = std::make_unique<SimpleThread>(m_archiveTcpClient);
	m_archiveTcpClientThread->start();

	connect(this, &MonitorTrendArchiveConnection::private_requestData,
			m_archiveTcpClient, &ArchiveTrendTcpClient::slot_requestData,
			Qt::QueuedConnection);

	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::dataReady, this, &MonitorTrendArchiveConnection::dataReady);
	connect(m_archiveTcpClient, &ArchiveTrendTcpClient::requestError, this, &MonitorTrendArchiveConnection::requestError);

	return;
}

MonitorTrendArchiveConnection::~MonitorTrendArchiveConnection()
{
	m_logFile->writeMessage(QString("TrendArchive %1 dtor.").arg(m_archiveServer.equipmentId));

	m_archiveTcpClientThread->quitAndWait(10000);

	return;
}

void MonitorTrendArchiveConnection::requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
												TimeStamp hourToRequest,
												E::TimeType timeType)
{
	Q_ASSERT(signalPlusServerId.archiveServerId == m_archiveServer.equipmentId);

	emit private_requestData(signalPlusServerId, hourToRequest, timeType);	// signal->slot passes requiest to the communication thread
	return;
}

const TrendLib::ArchiveServer& MonitorTrendArchiveConnection::archiveServer() const
{
	return m_archiveServer;
}

ArchiveTrendTcpClient::Stat MonitorTrendArchiveConnection::statistics() const
{
	return m_archiveTcpClient->stat();
}


//
//
//		MonitorTrendArchiveConnections
//
//
MonitorTrendArchiveConnections::MonitorTrendArchiveConnections(const MonitorConfigController& configController, ILogFile* logFile) :
	QObject(),
	m_configController(configController),
	m_logFile(logFile)
{
	return;
}

MonitorTrendArchiveConnections::~MonitorTrendArchiveConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return;
}

void MonitorTrendArchiveConnections::clear()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	m_createdConnectionsServers.clear();
	m_connections.clear();

	return;
}

void MonitorTrendArchiveConnections::createConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	clear();

	m_createdConnectionsServers = m_configController.configuration().archiveServices;

	const SoftwareInfo& softwareInfo = m_configController.softwareInfo();

	for (const MonitorSettings::ArchiveService& server : m_createdConnectionsServers)
	{
		MonitorTrendArchiveConnection& conn = m_connections.emplace_back(softwareInfo, server, m_logFile);

		connect(&conn, &MonitorTrendArchiveConnection::dataReady, this, &MonitorTrendArchiveConnections::dataReady);
		connect(&conn, &MonitorTrendArchiveConnection::requestError, this, &MonitorTrendArchiveConnections::requestError);
	}

	return;
}

void MonitorTrendArchiveConnections::updateConnections()
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	// New configuration arrived, server configuration could be changed,
	// if so, restart comminications threads.
	//
	m_logFile->writeMessage("TrendArchive: New configuration arrived.");

	auto archiveSecrvices = m_configController.configuration().archiveServices;

	if (std::ranges::equal(m_createdConnectionsServers, archiveSecrvices) == false)
	{
		clear();
		createConnections();
	}

	return;
}


size_t MonitorTrendArchiveConnections::size() const
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());
	return m_connections.size();
}

void MonitorTrendArchiveConnections::requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
												 TimeStamp hourToRequest,
												 E::TimeType timeType)
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	QString archievServerId = signalPlusServerId.archiveServerId;

	auto cit = std::find_if(m_connections.begin(), m_connections.end(),
				[&archievServerId](const auto& connection)
				{
					return connection.archiveServer().equipmentId == archievServerId;
				});

	if (cit == m_connections.end())
	{
		m_logFile->writeError(QString("Trend RequestData, archive server %1 for signal %2 not found.")
							  .arg(archievServerId, signalPlusServerId.appSignalId));
		return;
	}

	MonitorTrendArchiveConnection& connection = *cit;
	connection.requestData(signalPlusServerId, hourToRequest, timeType);

	return;
}

ArchiveTrendTcpClient::Stat MonitorTrendArchiveConnections::statistics() const
{
	Q_ASSERT(QThread::currentThreadId() == this->thread()->currentThreadId());

	ArchiveTrendTcpClient::Stat result{};

	for (const MonitorTrendArchiveConnection& connection : m_connections)
	{
		ArchiveTrendTcpClient::Stat cs = connection.statistics();

		if (cs.text.isEmpty() == false)
		{
			if (result.text.isEmpty() == false)
			{
				result.text += " | ";
			}

			result.text += cs.text;
		}

		result.replyCount += cs.replyCount;
		result.requestCount += cs.requestCount;
		result.requestQueueSize += cs.requestQueueSize;
		result.isConnected += cs.isConnected;
	}

	return result;
}
