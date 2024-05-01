#include "RtDataProviderPrivate.h"

namespace ClientLib
{
	RtConnection::RtConnection(const SoftwareInfo& softwareInfo,
							   SoftwareEndpoint::AppDataService server,
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

	const SoftwareEndpoint::AppDataService& RtConnection::server() const
	{
		return m_server;
	}

	RtTrendConnectionStatistics RtConnection::statistics() const
	{
		return m_rtTcpClient->stat();
	}

	//
	//
	//		RtDataProviderPrivate
	//
	//
	RtDataProviderPrivate::RtDataProviderPrivate(const ISignalDataServer& signalDataServer, ILogFile* logFile) :
		QObject{},
		m_signalDataServer(signalDataServer),
		m_logFile(logFile)
	{
		Q_ASSERT(m_logFile);
		return;
	}

	RtDataProviderPrivate::~RtDataProviderPrivate()
	{
		Q_ASSERT(QThread::currentThread() == this->thread());
		return;
	}

	void RtDataProviderPrivate::clear()
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		m_createdConnectionsServers.clear();
		m_connections.clear();

		return;
	}

	void RtDataProviderPrivate::createConnections(const SoftwareInfo& softwareInfo,
										   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		clear();

		m_createdConnectionsServers = appDataServices;

		for (const SoftwareEndpoint::AppDataService& server : m_createdConnectionsServers)
		{
			RtConnection& conn = m_connections.emplace_back(softwareInfo, server, m_signalDataServer, m_logFile);

			connect(&conn, &RtConnection::dataReady, this, &RtDataProviderPrivate::dataReady);
			connect(&conn, &RtConnection::requestError, this, &RtDataProviderPrivate::requestError);
			connect(&conn, &RtConnection::connectionLost, this, &RtDataProviderPrivate::connectionLost);
		}

		return;
	}

	void RtDataProviderPrivate::updateConnections(const SoftwareInfo& softwareInfo,
										   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		// New configuration arrived, server configuration could be changed,
		// if so, restart communications threads.
		//
		m_logFile->writeMessage("RtTrends: New configuration arrived.");

		if (std::ranges::equal(m_createdConnectionsServers, appDataServices) == false)
		{
			clear();
			createConnections(softwareInfo, appDataServices);
		}

		return;
	}

	bool RtDataProviderPrivate::setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		for (auto& conn : m_connections)
		{
			// Add all signals to connections, because possible the situation when signal were not downloaded yet from the server, and then
			// m_signalDataServer.dataServiceHasSignal cannot provide valid data, so, we set all signals and
			// in connection each time use only right signals
			//
			conn.setData(samplePeriod, trendSignals);
		}

		return true;
	}

	void RtDataProviderPrivate::setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		for (auto& conn : m_connections)
		{
			conn.setSamplePeriod(samplePeriod);
		}

		return;
	}

	size_t RtDataProviderPrivate::size() const
	{
		Q_ASSERT(QThread::currentThread() == this->thread());
		return m_connections.size();
	}

	RtTrendConnectionStatistics RtDataProviderPrivate::statistics() const
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		RtTrendConnectionStatistics result{};

		for (const auto& conn : m_connections)
		{
			RtTrendConnectionStatistics cs = conn.statistics();

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

	bool RtDataProviderPrivate::allConnected(std::chrono::milliseconds timeout) const
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		QDeadlineTimer timer{timeout};

		auto stats = statistics();
		while (stats.isConnected != size() && timer.hasExpired() == false)
		{
			QThread::msleep(timer.remainingTime() > 50 ? 50 : timer.remainingTime());
			stats = statistics();
		}

		return stats.isConnected == size();
	}
} // namespace ClientLib
