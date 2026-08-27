#include "SimulatorCommThread.h"

#include <RvduSimulatorLib.h>

constexpr size_t OptoBufferMaxSize = 2560 * 2;
constexpr size_t OptoPortCount = 8;

constexpr int MemoryUpdateTimeMs = 50;	// Memory update period
constexpr int StatsUpdateMs = 250;		// Statistics update period
constexpr int TimeoutMs = 3000;			// Timeout period

SimCommThreadWorker::SimCommThreadWorker(QString sharedMemoryKey) :
	m_sharedMemoryKey(sharedMemoryKey)
{
}

void SimCommThreadWorker::threadProc()
{
	while (QThread::currentThread()->isInterruptionRequested() == false)
	{
		processVduSharedData();

		QThread::msleep(MemoryUpdateTimeMs);
	}
}

void SimCommThreadWorker::processVduSharedData()
{
	if (m_sharedMemoryKey.isEmpty() == true)
	{
		emit updateSharedDataInfo("Autonomous Mode", false);
		return;
	}

	if (m_sharedMemory.has_value() == false)
	{
		initSharedMemory();
	}

	if (m_sharedMemory.has_value() == true)
	{
		m_sharedMemory->lock();

		processSharedMemory();

		m_sharedMemory->unlock();
	}
}

void SimCommThreadWorker::initSharedMemory()
{
	if (m_sharedMemory.has_value() == true) 
	{
		Q_ASSERT(false);
		return;
	}

	// Create shared memory for VDU simulator
	//
	m_sharedMemory.emplace(m_sharedMemoryKey);

	bool created = m_sharedMemory->create(sizeof(SimVduSharedData::SharedData), QSharedMemory::ReadWrite);
	if (created == false)
	{
		if (m_sharedMemory->error() != QSharedMemory::AlreadyExists)
		{
			logError(tr("Create shared memory error: %1").arg(m_sharedMemory->errorString()));
			m_sharedMemory.reset();
			return;
		}

		// If shared memory already exists, try to attach to it
		//
		bool attached = m_sharedMemory->attach(QSharedMemory::ReadWrite);
		if (attached == false)
		{
			logError(tr("Attach to existing shared memory error: %1").arg(m_sharedMemory->errorString()));
			m_sharedMemory.reset();
			return;
		}

		logMessage(tr("Shared memory was successfully attached."));
	}
	else
	{
		// Initialize the created data
		//
		auto sharedData = static_cast<SimVduSharedData::SharedData*>(m_sharedMemory->data());
		if (sharedData == nullptr)
		{
			logError(tr("Shared memory data pointer is null."));
			m_sharedMemory.reset();
			return;
		}

		// std::scoped_lock lock{*m_shared.clientSharedMemory};
		*sharedData = SimVduSharedData::SharedData{};
		sharedData->size = sizeof(SimVduSharedData::SharedData);

		logMessage(tr("Shared memory was successfully created and initialized."));
	}
}

void SimCommThreadWorker::processSharedMemory()
{
	assert(m_sharedMemory->isAttached() == true);

	auto sharedData = static_cast<SimVduSharedData::SharedData*>(m_sharedMemory->data());
	if (sharedData == nullptr)
	{
		logError(tr("Shared memory data pointer is null."));
		m_sharedMemory.reset();
		return;
	}

	if (sharedData->size != sizeof(SimVduSharedData::SharedData))
	{
		logError(tr("Shared memory data has wrong size."));
		return;
	}

	// Process statistics
	//
	static int counter = 0;
	if (counter++ >= (StatsUpdateMs / MemoryUpdateTimeMs))
	{
		counter = 0;

		QString deviceStatestr;

		if (m_timeout == true) 
		{
			deviceStatestr = "Timeout";
		}
		else
		{
			switch (sharedData->simulatorState.deviceState)
			{
			case static_cast<int>(DeviceState::Off):
				deviceStatestr = "Off";
				break;
			case static_cast<int>(DeviceState::Start):
				deviceStatestr = "Start";
				break;
			case static_cast<int>(DeviceState::Fault):
				deviceStatestr = "Fault";
				break;
			case static_cast<int>(DeviceState::Operate):
				deviceStatestr = "Operate";
				break;
			}
		}

		QString str =
			tr("%1, state: %2, counter: %3").arg(m_sharedMemoryKey).arg(deviceStatestr).arg(sharedData->simulatorState.updateCounter);

		emit updateSharedDataInfo(str, m_timeout);
	}

	// Update the contents
	//
	if (sharedData->simulatorState.updateCounter != m_prevUpdateCounter)
	{
		m_prevUpdateCounter = sharedData->simulatorState.updateCounter;

		m_timeout = false;
		m_timeoutCounter = 0;

		rnLibReadMemoryLock();
		for (int i = 0; i < OptoPortCount; i++)
		{

			//qDebug() << sharedData->toVdu.portData[i].occupied;

			// if (sharedData->toVdu.portData[i].timeout == false)
			{
				rnLibWriteReadMemory(i * OptoBufferMaxSize, sharedData->toVdu.portData[i].data, OptoBufferMaxSize);
			}
			// else
			//{
			//  clear on timeout?
			//}
		}
		rnLibReadMemoryUnlock();

		rnLibWriteMemoryLock();
		for (int i = 0; i < OptoPortCount; i++)
		{
			sharedData->fromVdu.updateCounter++;
			//if (sharedData->fromVdu.portData[i].timeout == false)
			{
				rnLibReadWriteMemory(i * OptoBufferMaxSize, OptoBufferMaxSize, sharedData->fromVdu.portData[i].data);
			}
			// else
			//{
			//  clear on timeout?
			//}
		}
		rnLibWriteMemoryUnlock();
	}
	else
	{
		if (m_timeout == false)
		{
			m_timeoutCounter++;

			if (m_timeoutCounter >= (TimeoutMs / MemoryUpdateTimeMs))
			{
				m_timeout = true;
			}
		}
	}
}

SimCommThreadController::SimCommThreadController(QObject* parent,
												 QString sharedMemoryKey) :
	QObject(parent)
{
	m_worker = new SimCommThreadWorker(sharedMemoryKey);
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(&m_workerThread, &QThread::started, m_worker, &SimCommThreadWorker::threadProc);
	m_worker->moveToThread(&m_workerThread);

	connect(m_worker, &SimCommThreadWorker::logError, this, &SimCommThreadController::logError);
	connect(m_worker, &SimCommThreadWorker::logWarning, this, &SimCommThreadController::logWarning);
	connect(m_worker, &SimCommThreadWorker::logMessage, this, &SimCommThreadController::logMessage);

	connect(m_worker, &SimCommThreadWorker::updateSharedDataInfo, this, &SimCommThreadController::updateSharedDataInfo);

	m_workerThread.start();
}

SimCommThreadController::~SimCommThreadController()
{
	m_workerThread.requestInterruption();
	m_workerThread.quit();
	m_workerThread.wait();
}
