#pragma once

#include <QObject>
#include <QThread>
#include <QSharedMemory>

#include "../SimulatorLib/include/SimulatorLib/SimVduSharedData.h"

 enum class DeviceState
{
	Off = 0,
	Start = 1,
	Fault = 2,
	Operate = 3
};

class SimCommThreadWorker : public QObject
{
	Q_OBJECT
public:
	explicit SimCommThreadWorker(QString sharedMemoryKey);

	bool isRunning() const;
	bool isError() const;

signals:
	void logError(const QString& msg);
	void logWarning(const QString& msg);
	void logMessage(const QString& msg);

	void updateSharedDataInfo(const QString& info, bool timeout);
	void bringToFront();

public slots:
	void threadProc();

private:
	void initOptoPortsCounters();

	void writeVduTime();
	void processVduSharedData();

	void initSharedMemory();
	void processSharedMemory();

	void processStatus(SimVduSharedData::SharedData* sharedData);
	void processRxOptoPorts(SimVduSharedData::SharedData* sharedData);
	void processTxOptoPorts(SimVduSharedData::SharedData* sharedData);
	void processInputDiscretes(SimVduSharedData::SharedData* sharedData);
	void processOutputDiscretes(SimVduSharedData::SharedData* sharedData);

	void clearInputValidity();

private:
	QString m_sharedMemoryKey;
	std::optional<QSharedMemory> m_sharedMemory;

	uint64_t m_timeoutCounter = 0;
	bool m_timeout = false;

	uint64_t m_outOptoOccupied[SimVduSharedData::OptoPortCount] = {0};
};

class SimCommThreadController : public QObject
{
	Q_OBJECT

public:
	SimCommThreadController(QObject* parent,
							QString sharedMemoryKey);
	~SimCommThreadController();

signals:
	void logError(const QString& msg);
	void logWarning(const QString& msg);
	void logMessage(const QString& msg);

	void updateProjectInfo(const QString& projectInfo);
	void updateSharedDataInfo(const QString& info, bool timeout);
	void bringToFront();

private:
	SimCommThreadWorker* m_worker = nullptr;
	QThread m_workerThread;
};

