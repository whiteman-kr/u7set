#pragma once

#include <QObject>
#include "../UtilsLib/ILogFile.h"
#include "../../lib/SoftwareSettings.h"
#include "ArchiveData.h"
#include "ArchiveTcpClient2.h"

class MonitorConfigController;


class ArchiveConnectionTask : public QObject
{
	Q_OBJECT

public:
	ArchiveConnectionTask(ArchiveSource request,
						  const SoftwareInfo& softwareInfo,
						  const MonitorSettings::ArchiveService& archiveService,
						  ILogFile* logFile);

	ArchiveConnectionTask() = delete;
	ArchiveConnectionTask(const ArchiveConnectionTask&) = delete;
	ArchiveConnectionTask(ArchiveConnectionTask&& src) = delete;
	ArchiveConnectionTask& operator=(const ArchiveConnectionTask&) = delete;
	ArchiveConnectionTask& operator=(ArchiveConnectionTask&&) = delete;

	~ArchiveConnectionTask();

public:
	bool dataReady() const;
	ArchiveRequestResult data();	// Throws std::runtime_error if communication error accured

private:
	MonitorSettings::ArchiveService m_archiveService;
	ArchiveTcpClient2* m_tcpClient = nullptr;
	SimpleThread* m_clientThread = nullptr;

	std::future<ArchiveRequestResult> m_future;
};


class ArchiveConnection : public QObject
{
	Q_OBJECT

public:
	explicit ArchiveConnection(MonitorConfigController& configController,
							   ILogFile* logFile,
							   QObject *parent = nullptr);
	virtual ~ArchiveConnection() = default;

public:
	// Request Data from archive
	//
	void request(ArchiveSource requestData);

	[[nodiscard]] bool requestInProgress() const;

//public slots:
//	void dataReady(std::shared_ptr<ArchiveChunk> chunk);
//	void statusUpdate(QString status, int statesReceived, int requestCount, int repliesCount);
//	void requestIsFinished();


signals:
	void startRequest(ArchiveSource requestData);
	void cancelRequest();
	void requestError(QString errorMessage);

	//	connect(m_tcpClient, &ArchiveTcpClient::dataReady, this, &MonitorArchiveWidget::dataReceived);
	//	connect(m_tcpClient, &ArchiveTcpClient::requestError, this, &MonitorArchiveWidget::tcpClientError);
	//	connect(m_tcpClient, &ArchiveTcpClient::statusUpdate, this, &MonitorArchiveWidget::tcpStatus);
	//	connect(m_tcpClient, &ArchiveTcpClient::requestIsFinished, this, &MonitorArchiveWidget::tcpRequestFinished);

private slots:
	void slot_startRequest(ArchiveSource requestData);
	void slot_cancelRequest();

private:
	const MonitorConfigController& m_configController;
	HasLogFile m_logFile;

	// This must be changed only from one thread (main thread)
	//
	ArchiveSource m_requestData;
	std::list<ArchiveConnectionTask> m_connections;
};

