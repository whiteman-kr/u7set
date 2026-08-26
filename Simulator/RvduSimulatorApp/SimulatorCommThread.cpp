#include "SimulatorCommThread.h"

#include <RvduSimulatorLib.h>

constexpr size_t OptoBufferMaxSize = 2560 * 2;
constexpr size_t OptoPortCount = 8;

SimCommThreadWorker::SimCommThreadWorker(QString sharedMemoryKey) :
	m_sharedMemoryKey(sharedMemoryKey)
{
}

void SimCommThreadWorker::threadProc()
{
	while (QThread::currentThread()->isInterruptionRequested() == false)
	{
		processVduSharedData();

		QThread::msleep(50);
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

		processSharedMemory(sharedData);
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

void SimCommThreadWorker::processSharedMemory(SimVduSharedData::SharedData* const sharedData)
{
	static int counter = 0;
	if (counter++ >= (250 /*250 ms*/ / 50))
	{
		counter = 0;

		QString deviceStatestr;

		if (m_sharedMemoryTimeout == true) 
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

		emit updateSharedDataInfo(str, m_sharedMemoryTimeout);
	}

	if (sharedData->simulatorState.updateCounter != m_prevUpdateCounter)
	{
		m_prevUpdateCounter = sharedData->simulatorState.updateCounter;

		m_sharedMemoryTimeout = false;
		m_sharedMemoryTimeoutCounter = 0;

		rnLibReadMemoryLock();
		for (int i = 0; i < OptoPortCount; i++)
		{
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
			if (sharedData->fromVdu.portData[i].timeout == false)
			{
				rnLibReadWriteMemory(i * OptoBufferMaxSize, sharedData->fromVdu.portData[i].data, OptoBufferMaxSize);
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
		if (m_sharedMemoryTimeout == false)
		{
			m_sharedMemoryTimeoutCounter++;

			if (m_sharedMemoryTimeoutCounter >= (3000 /*3 secs*/ / 50))
			{
				m_sharedMemoryTimeout = true;
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
