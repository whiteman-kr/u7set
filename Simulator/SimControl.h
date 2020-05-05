#ifndef SIMCONTROL_H
#define SIMCONTROL_H

#include <set>
#include <atomic>
#include <memory>
#include <vector>
#include <optional>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QMutex>
#include <SimOutput.h>
#include <SimLogicModule.h>
#include <SimTimeController.h>
#include <SimAppSignalManager.h>

namespace Sim
{
	using namespace std::literals::chrono_literals;

	class Simulator;
	struct SimControlRunStruct;

	enum class SimControlState
	{
		Stop,
		Run,
		Pause
	};

	// Internal struct must not be used anywhere in code except Sim::Control
	//
	struct SimControlRunStruct
	{
		SimControlRunStruct(const std::shared_ptr<LogicModule>& lm) :
			m_lm(lm)
		{
		}

		QFuture<bool> start(std::chrono::microseconds time)
		{
			bool reset = m_lastStartTime == 0us;

			m_lastStartTime = time;
			m_possibleToAdvanceTo = time;
			m_cyclesCounter ++;
			return m_lm->asyncRunCycle(time, m_cyclesCounter, reset);
		}

		bool afterWorkCycleTask(AppSignalManager& appSignalManager, TimeStamp plantTime, TimeStamp localTime, TimeStamp systemTime)
		{
			// Set LogicModule's RAM to Sim::AppSignalManager
			//
			appSignalManager.setData(equipmentId(), m_lm->ram(), plantTime, localTime, systemTime);

			return true;
		}


		const QString& equipmentId() const
		{
			return m_lm->equipmentId();
		}

		LogicModule* operator->()
		{
			return m_lm.get();
		}

		std::shared_ptr<LogicModule> m_lm;
		std::chrono::microseconds m_lastStartTime{0};
		std::chrono::microseconds m_possibleToAdvanceTo{0};
		qint64 m_cyclesCounter = 0;

		std::optional<QFuture<bool>> m_task;
	};


	struct ControlData
	{
		// Keep this struct simple, it should copy fast enough
		//
		std::vector<SimControlRunStruct> m_lms;			// LMs added to simulation
		SimControlState m_state = SimControlState::Stop;

		std::chrono::microseconds m_startTime = 0us;	// When simulation was started, it's computer time
		std::chrono::microseconds m_currentTime = 0us;	// Current time in simulation

		std::chrono::microseconds m_duration{0};		// Simulation is started for this time
														// if time < 0 then no time limit
														// if time == 0 then run one cycle
														// if time > 0 then run this time

		QDateTime currentTime() const
		{
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime);
			return QDateTime::fromMSecsSinceEpoch(ms.count(), Qt::UTC);
		}
	};


	class Control : public QThread, protected Output
	{
		Q_OBJECT

	public:
		explicit Control(Simulator* simualtor, QObject* parent = nullptr);
		virtual ~Control();

	public:
		void stopThread();
		void reset();

		bool addToRunList(const QString& equipmentId);
		int addToRunList(const QStringList& equipmentIds);

		void removeFromRunList(const QString& equipmentId);
		void removeFromRunList(const QStringList& equipmentIds);

		bool startSimulation(std::chrono::microseconds duration = -1us);
		void pause();
		void stop();

		ControlData controlData() const;
		void updateControlData(const ControlData& cd);

		SimControlState state() const;
		bool isRunning() const;

		std::chrono::microseconds duration() const;
		std::chrono::microseconds leftTime() const;

	signals:
		void stateChanged(SimControlState state);

	protected:
		virtual void run() override;

		bool processRun();

	private:
		Simulator* m_simulator = nullptr;

		// Start of access only with mutex
		// \/ \/ \/ \/ \/
		mutable QReadWriteLock m_controlDataLock{QReadWriteLock::Recursive};
		ControlData m_controlData;
		// /\ /\ /\ /\ /\
		// End of Access only with mutex
		//
	};

}

#endif // SIMCONTROL_H
