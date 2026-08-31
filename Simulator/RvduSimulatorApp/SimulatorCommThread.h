#pragma once

#include <QObject>
#include <QThread>
#include <QSharedMemory>

//#include "../SimulatorLib/include/SimulatorLib/SimDeviceState.h"
//#include "../SimulatorLib/src/Modules/SimVduDeviceEmulator.h"

 enum class DeviceState
{
	Off = 0,
	Start = 1,
	Fault = 2,
	Operate = 3
};


namespace SimVduSharedData
{
	constexpr size_t OptoBufferMaxSize = 2560 * 2;
	constexpr size_t OptoPortCount = 8;

	struct SimulatorState
	{
		uint64_t updateCounter; // The update counter, incremented by simulator on each update
		int32_t deviceState;    // enum class DeviceState (0 - Off, Start, Faut, Operate)
		int32_t reserved0;      // Reserved for future use (padding to 8 bytes)
		uint64_t plantTime;     // The plant time in microseconds
	};

	struct OptoBuffer
	{
		uint64_t timeout;       //
		uint64_t occupied;      // The number of bytes occupied in the buffer
		uint8_t data[OptoBufferMaxSize];
	};

	struct OptoPorts
	{
		uint64_t updateCounter;
		OptoBuffer portData[OptoPortCount]; // Data from simulator to VDU
	};

	struct SharedData
	{
		uint64_t size;                      // The size of this structure, must be filled first

		SimulatorState simulatorState;      // State of the simulator

		OptoPorts toVdu;                    // Data from simulator to VDU
		OptoPorts fromVdu;                  // Data from VDU to simulator
	};

	static_assert(std::is_standard_layout_v<SharedData> && std::is_trivial_v<SharedData>, "SharedData must be POD type");
} // namespace SimVduSharedData


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

public slots:
	void threadProc();

private:
	void initOptoPortsCounters();

	void processVduSharedData();

	void initSharedMemory();
	void processSharedMemory();

private:
	QString m_sharedMemoryKey;
	std::optional<QSharedMemory> m_sharedMemory;

	uint64_t m_prevUpdateCounter = (uint64_t)-1;
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

private:
	SimCommThreadWorker* m_worker = nullptr;
	QThread m_workerThread;
};

