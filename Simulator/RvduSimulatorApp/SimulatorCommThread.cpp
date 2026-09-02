#include "SimulatorCommThread.h"

#include <RvduSimulatorLib.h>

constexpr int MemoryUpdateTimeMs = 50;	// Memory update period
constexpr int StatsUpdateMs = 250;		// Statistics update period
constexpr int TimeoutMs = 3000;			// Timeout period

SimCommThreadWorker::SimCommThreadWorker(QString sharedMemoryKey) :
	m_sharedMemoryKey(sharedMemoryKey)
{
}

void SimCommThreadWorker::threadProc()
{
	initOptoPortsCounters();

	while (QThread::currentThread()->isInterruptionRequested() == false)
	{
		processVduSharedData();

		QThread::msleep(MemoryUpdateTimeMs);
	}
	
	return;
}

void SimCommThreadWorker::initOptoPortsCounters() 
{ 
	// Each connection has Port ID at start
	//
	for (int p = 0; p < SimVduSharedData::OptoPortCount; p++) 
	{
		constexpr uint64_t PortIdSizeInWords = 2;
		m_outOptoOccupied[p] = PortIdSizeInWords * 2;
	}
	
	unsigned int count = rnLibAppSignalsCount();
	for (unsigned int si = 0; si < count; si++) 
	{
		if (rnLibIsTxSignal(si) == false) 
		{
			continue;
		}

		int pi = rnLibTxPortIndex(si);
		if (pi < 0 || pi >= SimVduSharedData::OptoPortCount) 
		{
			Q_ASSERT(false);
			return;
		}
		
		uint64_t offset = rnLibTxSignalOffset(si);
		
		constexpr uint64_t DiscreteSizeInWords = 1;
		constexpr uint64_t AnalogSizeInWords = 2;
		uint64_t sizeInWords = (rnLibIsDiscrete(si) == true) ? DiscreteSizeInWords : AnalogSizeInWords; // SI and FP occupy 2 words

		if (m_outOptoOccupied[pi] < (offset + sizeInWords) * 2) 
		{
			m_outOptoOccupied[pi] = (offset + sizeInWords) * 2;
		}
	}

	return;
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
		std::scoped_lock lock{*m_sharedMemory};
		processSharedMemory();
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

		{
			std::scoped_lock lock{*m_sharedMemory};
			*sharedData = SimVduSharedData::SharedData{};
			sharedData->size = sizeof(SimVduSharedData::SharedData);
		}

		logMessage(tr("Shared memory was successfully created and initialized."));
	}
}

#define ADDR_OPTO_VALIDITY 20491u
#define ADDR_DISCRETE_INPUTS 20481u
#define ADDR_DISCRETE_INPUTS_VALIDITY 20480u
#define ADDR_DISCRETE_OUTPUTS 20483u

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

	if (sharedData->simulatorState.bringToFront != 0) 
	{
		sharedData->simulatorState.bringToFront = 0;
		emit bringToFront();
	}

	processStatus(sharedData);

	static uint64_t m_prevSimulatorUpdateCounter = (uint64_t)-1;
	if (sharedData->simulatorState.updateCounter != m_prevSimulatorUpdateCounter)
	{
		m_prevSimulatorUpdateCounter = sharedData->simulatorState.updateCounter;

		m_timeout = false;
		m_timeoutCounter = 0;

		if (sharedData->simulatorState.deviceState == static_cast<int>(DeviceState::Operate))
		{
			// Update the Read memory contents

			rnLibReadMemoryLock();

			processInputDiscretes(sharedData);

			processRxOptoPorts(sharedData);

			processOutputDiscretes(sharedData);

			rnLibReadMemoryUnlock();

			// Update the Write memory contents

			rnLibWriteMemoryLock();

			processTxOptoPorts(sharedData);

			rnLibWriteMemoryUnlock();
		}
		else
		{
			clearInputValidity();
		}
	}
	else
	{
		// Process possible timeout
		//
		if (m_timeout == false)
		{
			m_timeoutCounter++;

			if (m_timeoutCounter >= (TimeoutMs / MemoryUpdateTimeMs))
			{
				m_timeout = true;
				clearInputValidity();
			}
		}
	}
}

void SimCommThreadWorker::processStatus(SimVduSharedData::SharedData* sharedData)
{
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
}

void SimCommThreadWorker::processRxOptoPorts(SimVduSharedData::SharedData* sharedData)
{
	// Read optical buffers
	//
	uint16_t optoValidity = 0;

	for (int i = 0; i < SimVduSharedData::OptoPortCount; i++)
	{
		if (sharedData->toVdu.portData[i].timeout == 0)
		{
			if (sharedData->toVdu.portData[i].occupied != 0)
			{
				uint8_t reversedData[SimVduSharedData::OptoBufferMaxSize];
				memcpy(reversedData, sharedData->toVdu.portData[i].data, SimVduSharedData::OptoBufferMaxSize);

				// Reverse bytes in words
				//
				uint16_t* data16 = (uint16_t*)reversedData;
				for (int j = 0; j < SimVduSharedData::OptoBufferMaxSize / 2; j++)
				{
					*data16 = qbswap(*data16);
					data16++;
				}

				rnLibWriteReadMemory(i * SimVduSharedData::OptoBufferMaxSize, reversedData, SimVduSharedData::OptoBufferMaxSize);

				optoValidity |= (1 << i);
			}
		}

		// Write optical ports validity

		rnLibWriteReadMemory(ADDR_OPTO_VALIDITY * 2, (unsigned char*)&optoValidity, sizeof(uint16_t));
	}
}

void SimCommThreadWorker::processTxOptoPorts(SimVduSharedData::SharedData* sharedData)
{ 
	// Write the optical ports output data
	//
	sharedData->fromVdu.updateCounter++;

	for (int i = 0; i < SimVduSharedData::OptoPortCount; i++)
	{
		sharedData->fromVdu.portData[i].timeout = 0;
		sharedData->fromVdu.portData[i].occupied = m_outOptoOccupied[i];
	
		rnLibReadWriteMemory(i * SimVduSharedData::OptoBufferMaxSize,
							 SimVduSharedData::OptoBufferMaxSize,
							 sharedData->fromVdu.portData[i].data);

		// Reverse bytes in words
		//
		uint16_t* data16 = (uint16_t*)sharedData->fromVdu.portData[i].data;
		for (int j = 0; j < SimVduSharedData::OptoBufferMaxSize / 2; j++)
		{
			*data16 = qbswap(*data16);
			data16++;
		}

		// Write tx ID
		//
		uint32_t txId = rnLibOptoPortTxDataID(i);
		*(uint32_t*)sharedData->fromVdu.portData[i].data = qToBigEndian(txId);

	}
}

void SimCommThreadWorker::processInputDiscretes(SimVduSharedData::SharedData* sharedData)
{  
	// Read input discretes
	//
	uint8_t diValue = 0;
	uint8_t diValidity = 0;

	static uint64_t m_prevInputDiscreteUpdateCounter = (uint64_t)-1;
	if (sharedData->discreteInputs.updateCounter != m_prevInputDiscreteUpdateCounter)
	{
		m_prevInputDiscreteUpdateCounter = sharedData->discreteInputs.updateCounter;
		for (int i = 0; i < SimVduSharedData::DiscreteInputCount; i++)
		{
			if (sharedData->discreteInputs.data[i] != 0)
			{
				diValue |= (1 << i);
			}
			if (sharedData->discreteInputs.validity[i] != 0)
			{
				diValidity |= (1 << i);
			}
		}
	}
	rnLibWriteReadMemory(ADDR_DISCRETE_INPUTS * 2, (unsigned char*)&diValue, sizeof(uint8_t));
	rnLibWriteReadMemory(ADDR_DISCRETE_INPUTS_VALIDITY * 2, (unsigned char*)&diValidity, sizeof(uint8_t));
}

void SimCommThreadWorker::processOutputDiscretes(SimVduSharedData::SharedData* sharedData)
{ 
	// Write the output discretes state
	//
	uint8_t doValue = 0;
	rnLibReadReadMemory(ADDR_DISCRETE_OUTPUTS * 2, sizeof(uint8_t), &doValue);

	sharedData->discreteOutputs.updateCounter++;

	for (int i = 0; i < SimVduSharedData::DiscreteOutputCount; i++)
	{
		if ((doValue & (1 << i)) != 0)
		{
			sharedData->discreteOutputs.data[i] = 1;
		}
		else
		{
			sharedData->discreteOutputs.data[i] = 0;
		}
	}
}

void SimCommThreadWorker::clearInputValidity()
{ 
	// Clear optical ports validity
	//
	uint16_t optoValidity = 0;
	rnLibWriteReadMemory(ADDR_OPTO_VALIDITY * 2, (unsigned char*)&optoValidity, sizeof(uint16_t));

	// Clear input discretes validity
	//
	uint8_t diValidity = 0;
	rnLibWriteReadMemory(ADDR_DISCRETE_INPUTS_VALIDITY * 2, (unsigned char*)&diValidity, sizeof(uint8_t));
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
	connect(m_worker, &SimCommThreadWorker::bringToFront, this, &SimCommThreadController::bringToFront);

	m_workerThread.start();
}

SimCommThreadController::~SimCommThreadController()
{
	m_workerThread.requestInterruption();
	m_workerThread.quit();
	m_workerThread.wait();
}
