#include "ArchiveConnection.h"
#include "ArchiveTcpClient2.h"
#include "../../UtilsLib/SimpleThread.h"
#include "../MonitorConfigController.h"


ArchiveConnectionTask::ArchiveConnectionTask(ArchiveSource request,
											 const SoftwareInfo& softwareInfo,
											 const MonitorSettings::ArchiveService& archiveService,
											 ILogFile* logFile) :
	m_archiveService(archiveService),
	m_tcpClient(new ArchiveTcpClient2(request, softwareInfo, archiveService, logFile)),
	m_clientThread(new SimpleThread(m_tcpClient)),
	m_future(m_tcpClient->future())
{
	m_clientThread->start();
	return;
}

ArchiveConnectionTask::~ArchiveConnectionTask()
{
	Q_ASSERT(m_clientThread);

	if (m_clientThread != nullptr)
	{
		m_tcpClient->cancelRequest();
		m_future.wait_for(std::chrono::seconds(5));

		m_clientThread->quitAndWait(5000);
		delete m_clientThread;
	}

	return;
}

bool ArchiveConnectionTask::dataReady() const
{
	Q_ASSERT(m_future.valid() == true);
	return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

ArchiveRequestResult ArchiveConnectionTask::data()
{
	Q_ASSERT(m_future.valid() == true);
	Q_ASSERT(dataReady() == true);
	return m_future.get();
}

ArchiveConnection::ArchiveConnection(MonitorConfigController& configController,
									 ILogFile* logFile,
									 QObject* parent) :
	QObject(parent),
	m_configController(configController),
	m_logFile(logFile, "ArchiveConnection")
{
	qRegisterMetaType<ArchiveSource>("ArchiveSource");

	connect(this, &ArchiveConnection::startRequest, this, &ArchiveConnection::slot_startRequest, Qt::QueuedConnection);
	connect(this, &ArchiveConnection::cancelRequest, this, &ArchiveConnection::slot_cancelRequest, Qt::QueuedConnection);

	return;
}

void ArchiveConnection::request(ArchiveSource requestData)
{
	emit startRequest(std::move(requestData));	// emit signal as requestData func can be called from other thread
	return;
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
							.arg(requestData.requestStartTime.toDateTime().toString())
							.arg(requestData.requestEndTime.toDateTime().toString())
							.arg(E::valueToString(requestData.timeType))
							.arg(requestData.removePeriodicRecords)
							.arg([](const auto& archSignals) -> QString
									{
										QStringList result;
										result.reserve(static_cast<int>(archSignals.size()));
										for (const ArchiveSignal& s : archSignals)
										{
											result.push_back(s.signalParam.appSignalId());
										}
										return result.join(", ");
									}(requestData.acceptedSignals))
						   );

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
	std::map<QString, std::vector<ArchiveSignal>> m_tasks;		// Key is ArchiveServiceId

	for (const ArchiveSignal& s : m_requestData.acceptedSignals)
	{
		m_tasks[s.archiveServiceId].push_back(s);
	}

	// Add tasks splitted by archive services
	//
	auto archiveService = m_configController.configuration().archiveServices;

	for (const auto&[archiveServiceId, archiveSignals] : m_tasks)
	{
		ArchiveSource request = m_requestData;
		request.acceptedSignals = archiveSignals;		// Set signals from this ArchiveService

		// Find ArchiveService configuration
		//
		auto archServiceIt = std::find_if(archiveService.begin(), archiveService.end(),
								[archiveServiceId](const MonitorSettings::ArchiveService& as)
								{
									return as.equipmentId == archiveServiceId;
								});

		if (archServiceIt == archiveService.end())
		{
			QString error = tr("Cannot start request to as configuration for archive service %1 not found.").arg(archiveServiceId);
			m_logFile.writeError(error);
			emit requestError(error);
			continue;
		}

		// Create connections
		//
		m_connections.emplace_back(std::move(request),
								   m_configController.softwareInfo(),
								   *archServiceIt,
								   m_logFile.logFile());
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

	m_connections.clear();

	return;
}

