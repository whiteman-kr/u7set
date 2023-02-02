#include "ArchiveConnection.h"
#include "ArchiveTcpClient2.h"
#include "../MonitorConfigController.h"

ArchiveConnectionTask::ArchiveConnectionTask(ArchiveSource request,
											 const SoftwareInfo& softwareInfo,
											 const MonitorSettings::ArchiveService& archiveService,
											 ILogFile* logFile)	:
	m_archiveService(archiveService),
	m_tcpClient(new ArchiveTcpClient2(request, softwareInfo, archiveService, logFile)),
	m_clientThread(new SimpleThread(m_tcpClient))
{
	qDebug() << Q_FUNC_INFO << ", AcrhiveService: " << m_archiveService.shortenId;

	qRegisterMetaType<ArchiveRequestResult>();
	qRegisterMetaType<std::shared_ptr<ArchiveRequestResult>>();

	m_clientThread->start();

	connect(m_tcpClient, &ArchiveTcpClient2::dataReady, this, &ArchiveConnectionTask::dataReady);

	return;
}

ArchiveConnectionTask::~ArchiveConnectionTask()
{
	qDebug() << Q_FUNC_INFO << ", AcrhiveService: " << m_archiveService.shortenId;

	Q_ASSERT(m_clientThread);

	if (m_clientThread != nullptr)
	{
		m_tcpClient->cancelRequest();
		m_clientThread->quitAndWait(5000);

		delete m_clientThread;
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
	emit slot_cancelRequest();
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
		auto searchPred = [archsrv = archiveServiceId](const MonitorSettings::ArchiveService& as)
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

	m_connections.clear();

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
	// Ñheck taht slot is in right thread as m_connections is not protected with mutex
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
