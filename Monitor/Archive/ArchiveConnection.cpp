#include "ArchiveConnection.h"
#include "ArchiveTcpClient2.h"
#include "../MonitorConfigController.h"

ArchiveConnectionTask::ArchiveConnectionTask(ArchiveSource request,
											 const SoftwareInfo& softwareInfo,
											 const SoftwareEndpoint::ArchiveService& archiveService,
											 ILogFile* logFile)	:
	m_archiveService(archiveService),
	m_tcpClient(new ArchiveTcpClient2(request, softwareInfo, archiveService, logFile)),
	m_clientThread(new SimpleThread(m_tcpClient))
{
	qDebug() << Q_FUNC_INFO << ", AcrhiveService: " << m_archiveService.shortenId;

	qRegisterMetaType<ArchiveRequestResult>();
	qRegisterMetaType<std::shared_ptr<ArchiveRequestResult>>();

	m_clientThread->start();

	connect(m_tcpClient, &ArchiveTcpClient2::statistics, this, &ArchiveConnectionTask::statistics);
	connect(m_tcpClient, &ArchiveTcpClient2::dataReady, this, &ArchiveConnectionTask::dataReady);

	return;
}

ArchiveConnectionTask::~ArchiveConnectionTask()
{
	qDebug() << Q_FUNC_INFO << ", AcrhiveService: " << m_archiveService.shortenId;

	Q_ASSERT(m_clientThread);

	if (m_clientThread != nullptr)
	{
		m_clientThread->quitAndWait(5000);

		delete m_clientThread;
	}

	return;
}

void ArchiveConnectionTask::cancelRequest()
{
	if (m_tcpClient != nullptr)
	{
		m_tcpClient->cancelRequest();
	}

	return;
}

QString ArchiveConnectionTask::archiveServiceId() const
{
	return m_archiveService.equipmentId;
}

HostAddressPort ArchiveConnectionTask::address() const
{
	return m_archiveService.address;
}

ArchiveConnection::ArchiveConnection(const MonitorConfigController& configController,
									 ILogFile* logFile,
									 QObject* parent) :
	QObject(parent),
	m_configController(configController),
	m_logFile(logFile, "ArchiveConnection")
{
	qRegisterMetaType<ArchiveSource>("ArchiveSource");

	connect(this, &ArchiveConnection::private_startRequest, this, &ArchiveConnection::slot_startRequest, Qt::QueuedConnection);
	connect(this, &ArchiveConnection::private_cancelRequest, this, &ArchiveConnection::slot_cancelRequest, Qt::QueuedConnection);

	return;
}

void ArchiveConnection::request(ArchiveSource requestData)
{
	{
		m_statisticsMutext.lock();
		m_statistics.clear();
		m_statisticsMutext.unlock();
	}

	if (requestData.acceptedSignals.size() > ARCH_REQUEST_MAX_SIGNALS)
	{
		QString errorMessage = QString("requestData() appSignals.size()(%1) > ARCH_REQUEST_MAX_SIGNALS(%2), cancel request")
							   .arg(requestData.acceptedSignals.size())
							   .arg(ARCH_REQUEST_MAX_SIGNALS);

		m_logFile.writeWarning(errorMessage);
		return;
	}

	// emit signal as requestData func can be called from other thread
	//
	emit private_startRequest(std::move(requestData));
	return;
}

void ArchiveConnection::cancelRequest()
{
	emit private_cancelRequest();
}

bool ArchiveConnection::requestInProgress() const
{
	Q_ASSERT(this->thread() == QThread::currentThread());
	return m_connections.empty() == false;
}

void ArchiveConnection::slot_startRequest(ArchiveSource requestData)
{
	Q_ASSERT(this->thread() == QThread::currentThread());

	if (requestData.acceptedSignals.size() > ARCH_REQUEST_MAX_SIGNALS)
	{
		QString errorMessage = QString("requestData() appSignals.size()(%1) > ARCH_REQUEST_MAX_SIGNALS(%2), cancel request")
							   .arg(requestData.acceptedSignals.size())
							   .arg(ARCH_REQUEST_MAX_SIGNALS);

		m_logFile.writeWarning(errorMessage);
		emit requestError(errorMessage);

		return;
	}

	if (requestInProgress() == true)
	{
		slot_cancelRequest();
	}
	m_logFile.writeMessage(QString("requestData(), startTime %1, endTime %2, timeType %3, removePeriodicRecords %4, appSignals: %5")
							   .arg(DateTimeToString::dateTimeSec(requestData.requestStartTime.toDateTime()))
							   .arg(DateTimeToString::dateTimeSec(requestData.requestEndTime.toDateTime()))
						   .arg(E::valueToString(requestData.timeType))
						   .arg(requestData.removePeriodicRecords)
						   .arg([](const auto &archSignals) -> QString
	{
		QStringList result;
		result.reserve(static_cast<int>(archSignals.size()));
		for (const ArchiveSignal &s : archSignals)
		{
			result.push_back(s.signalParam.appSignalId());
		}
		return result.join(", ");
	}(requestData.acceptedSignals)));

	// --
	//
	m_requestData = std::move(requestData);

	m_requestData.requestStartTime = qMin(m_requestData.requestStartTime, m_requestData.requestEndTime);
	m_requestData.requestEndTime = qMax(m_requestData.requestStartTime, m_requestData.requestEndTime);

	if (m_requestData.acceptedSignals.empty() == true)
	{
		Q_ASSERT(m_requestData.acceptedSignals.empty() == false);
		return;
	}

	// Split all signals by archive services
	//
	std::map<QString, std::vector<ArchiveSignal>> tasks; // Key is ArchiveServiceId

	for (const ArchiveSignal &s : m_requestData.acceptedSignals)
	{
		tasks[s.archiveServiceId].push_back(s);
	}

	// Add tasks splitted by archive services
	// Make a copy, as m_configController.configuration() is under mutext and returns copy
	//
	const auto archiveService = m_configController.configuration().archiveServices;

	for (const auto& [archiveServiceId, archiveSignals] : tasks)
	{
		ArchiveSource request = m_requestData;
		request.acceptedSignals = archiveSignals; // Set signals from this ArchiveService

		// Find ArchiveService configuration
		//
		auto searchPred = [archsrv = archiveServiceId](const SoftwareEndpoint::ArchiveService& as)
		{
			return as.equipmentId == archsrv;
		};

		auto archServiceIt = std::find_if(archiveService.begin(),
										  archiveService.end(),
										  searchPred);

		if (archServiceIt == archiveService.end())
		{
			QString error = tr("Cannot start request to as configuration for archive service %1 not found.")
							.arg(archiveServiceId);
			m_logFile.writeError(error);
			emit requestError(error);
			continue;
		}

		// Create connections
		//
		ArchiveConnectionTask& conn = m_connections.emplace_back(std::move(request),
																 m_configController.softwareInfo(),
																 *archServiceIt,
																 m_logFile.logFile());

		connect(&conn, &ArchiveConnectionTask::statistics, this, &ArchiveConnection::slot_statistics);
		connect(&conn, &ArchiveConnectionTask::dataReady, this, &ArchiveConnection::slot_taskDataReady);
	}

	return;
}

void ArchiveConnection::slot_cancelRequest()
{
	Q_ASSERT(this->thread() == QThread::currentThread());

	if (requestInProgress() == false)
	{
		return;
	}

	m_logFile.writeMessage(tr("Cancel %1 archive request connections.").arg(m_connections.size()));

	for (auto& conn : m_connections)
	{
		conn.cancelRequest();
	}

	m_connections.clear();
	emitStatistics();

	return;
}

void ArchiveConnection::slot_statistics(QString archServiceShortId, QString state, int requests, int replies, int states)
{
	{
		QMutexLocker locker(&m_statisticsMutext);

		m_statistics[archServiceShortId] =
			{
				.serviceId = archServiceShortId,
				.state = state,
				.requests = requests,
				.replies = replies,
				.states = states
			};
	}

	emitStatistics();

	return;
}

void ArchiveConnection::slot_taskDataReady(std::shared_ptr<ArchiveRequestResult> result, QString error)
{
	if (error.isEmpty() == true)
	{
		emit dataReady(result);
	}
	else
	{
		emit requestError(error);
		cancelRequest();	// cancelRequest() clears m_connections
		return;
	}

	// Task is done, remove it
	// Check that slot is in the right thread as m_connections is not protected with mutex
	//
	Q_ASSERT(this->thread() == QThread::currentThread());

	QString archiveServiceId = result->archiveServiceId;
	Q_ASSERT(archiveServiceId.isEmpty() == false);

	auto cmpfunc = [&archiveServiceId](const ArchiveConnectionTask& task)
	{
		return task.archiveServiceId() == archiveServiceId;
	};

	std::size_t removed = m_connections.remove_if(cmpfunc);

	Q_ASSERT(removed == 1);
	Q_UNUSED(removed);

	if (m_connections.empty() == true)
	{
		// All tasks are done
		//
		emit done();
	}

	return;
}

void ArchiveConnection::emitStatistics()
{
	QString totalServersSatus;
	int totalRequests = 0;
	int totalReplies = 0;
	int totalStates = 0;

	{
		QMutexLocker locker(&m_statisticsMutext);

		for (const auto& [serverId, stats] : m_statistics)
		{
			if (totalServersSatus.isEmpty() == true)
			{
				totalServersSatus = serverId + ": " + stats.state;
			}
			else
			{
				totalServersSatus += ", " + serverId + ": " + stats.state;
			}

			totalRequests += stats.requests;
			totalReplies += stats.replies;
			totalStates += stats.states;
		}
	}

	emit stats(totalServersSatus, totalRequests, totalReplies, totalStates);
	return;
}
