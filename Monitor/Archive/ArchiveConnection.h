#pragma once

#include <QObject>
#include "../../UtilsLib/ILogFile.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "ArchiveData.h"
#include "ArchiveTcpClient2.h"

class MonitorConfigController;


class ArchiveConnectionTask : public QObject
{
	Q_OBJECT

public:
	ArchiveConnectionTask(ArchiveSource request,
						  const SoftwareInfo& softwareInfo,
						  const SoftwareEndpoint::ArchiveService& archiveService,
						  ILogFile* logFile);

	ArchiveConnectionTask() = delete;
	ArchiveConnectionTask(const ArchiveConnectionTask&) = delete;
	ArchiveConnectionTask(ArchiveConnectionTask&& src) = delete;
	ArchiveConnectionTask& operator=(const ArchiveConnectionTask&) = delete;
	ArchiveConnectionTask& operator=(ArchiveConnectionTask&&) = delete;

	virtual ~ArchiveConnectionTask();

public:
	void cancelRequest();

	[[nodiscard]] QString archiveServiceId() const;
	[[nodiscard]] HostAddressPort address() const;

signals:
	void statistics(QString archServiceShortId, QString state, int requests, int replies, int states);
	void dataReady(std::shared_ptr<ArchiveRequestResult> result, QString error);

private:
	SoftwareEndpoint::ArchiveService m_archiveService;
	ArchiveTcpClient2* m_tcpClient = nullptr;
	SimpleThread* m_clientThread = nullptr;
};


class ArchiveConnection : public QObject
{
	Q_OBJECT

public:
	explicit ArchiveConnection(const MonitorConfigController& configController,
							   ILogFile* logFile,
							   QObject* parent = nullptr);
	virtual ~ArchiveConnection() = default;

public:
	/// Request Data from archive.
	void request(ArchiveSource requestData);

	/// Cancel request if any in progress.
	void cancelRequest();

	/// Check if request in progress
	[[nodiscard]] bool requestInProgress() const;

signals:
	// Use these there signals to control requests to ArchiveService
	//
	void dataReady(std::shared_ptr<ArchiveRequestResult> result);	///< Archive request result is reqdy
	void requestError(QString errorMessage);						///< Emited when request has an error
	void done();													///< All tasks are done, request finished

	void stats(QString serverStatus, int requests, int replies, int states);	///< Reports connection statistics

	// Private signals, used for inner communications
	//
	void private_startRequest(ArchiveSource requestData);	///< Start request command, can be issued from any thread
	void private_cancelRequest();							///< Cancel request command, can be issued from any thread

private slots:
	void slot_startRequest(ArchiveSource requestData);
	void slot_cancelRequest();
	void slot_statistics(QString archServiceShortId, QString state, int requests, int replies, int states);

	void slot_taskDataReady(std::shared_ptr<ArchiveRequestResult> result, QString error);

private:
	void emitStatistics();

private:
	const MonitorConfigController& m_configController;
	HasLogFile m_logFile;

	// This must be changed only from one thread (main thread)
	//
	ArchiveSource m_requestData;
	std::list<ArchiveConnectionTask> m_connections;

	// --
	//
	struct Statistics
	{
		QString serviceId;
		QString state;
		int requests = 0;
		int replies = 0;
		int states = 0;
	};

	QMutex m_statisticsMutext;
	std::map<QString, Statistics> m_statistics;
};

