#pragma once

#include <chrono>
#include <set>
#include <atomic>
#include <memory>
#include <vector>
#include <optional>
#include <mutex>
#include <condition_variable>
#include <QThread>
#include <QtConcurrent>
#include <QMutex>
#include "SimLogicModule.h"


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
		SimControlRunStruct(std::shared_ptr<LogicModule> lm) :
			m_lm(std::move(lm))
		{
		}

		QFuture<bool> start(std::chrono::microseconds time, const QDateTime& currentDateTime, std::condition_variable& cvFinished)
		{
			bool reset = m_lastStartTime == 0us;

			m_lastStartTime = time;
			m_possibleToAdvanceTo = time;
			m_cyclesCounter ++;
			return m_lm->asyncRunCycle(time, currentDateTime, m_cyclesCounter, reset, cvFinished);
		}

		const QString& equipmentId() const
		{
			return m_lm->equipmentId();
		}

		LogicModule* operator->()
		{
			return m_lm.get();
		}

		const LogicModule* operator->() const
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

		std::chrono::microseconds m_startTime = 0us;		// When simulation was started, computer time
		std::chrono::microseconds m_sliceStartTime = 0us;	// When simulation was started for current 'slice' (duration)
		std::chrono::microseconds m_currentTime = 0us;		// Current time in simulation

		std::chrono::microseconds m_duration{0};		// Simulation is started for this time
														// if time < 0 then no time limit
                                                        // if time == 0 then run one cycle (NO, IT WILL RESET IF ON PAUSE MODE)
														// if time > 0 then run this time

		QDateTime currentTime() const
		{
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime);
			return QDateTime::fromMSecsSinceEpoch(ms.count(), Qt::UTC);
		}
	};

	struct ControlStatus
	{
		ControlStatus() = default;

		ControlStatus(const ControlData& cd) :
			m_startTime(cd.m_startTime),
			m_currentTime(cd.m_currentTime),
			m_duration(cd.m_currentTime - cd.m_startTime),
			m_state(cd.m_state)
		{
			m_lmDeviceModes.reserve(cd.m_lms.size());

			for (const SimControlRunStruct& lm : cd.m_lms)
			{
				m_lmDeviceModes.push_back(Sim::ControlStatus::LmMode{lm.equipmentId(), lm.m_lm->deviceState()});
			}
		}

		std::chrono::microseconds m_startTime = 0us;	// When simulation was started, it's computer time
		std::chrono::microseconds m_currentTime = 0us;	// Current time in simulation

		std::chrono::microseconds m_duration{0};
		SimControlState m_state = SimControlState::Stop;

		struct LmMode
		{
			QString lmEquipmentId;
			Sim::DeviceState deviceState;
		};

		std::vector<Sim::ControlStatus::LmMode> m_lmDeviceModes;
	};


	class Control : public QThread
	{
		Q_OBJECT

	public:
		explicit Control(Simulator* simualtor, QObject* parent = nullptr);
		virtual ~Control();

	public:
		void stopThread();
		void reset();

		int setRunList(QStringList equipmentIds);

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

		double speedFactor() const;
		void setSpeedFactor(double value);

	signals:
		void stateChanged(SimControlState state);
		void statusUpdate(ControlStatus state);

	private:
		virtual void run() override;
		bool processRun();

	private:
		Simulator* m_simulator = nullptr;
		ScopedLog m_log;

		std::atomic<double> m_speedFactor;

		// m_insideProcessRun indicates that simulation thread now in the function processRun(),
		// while we are in this function we cannot do some operations, like m_simulator->software().stopSimulation().
		//
		std::atomic<bool> m_insideProcessRun{false};

		// Start of access only with mutex
		// \/ \/ \/ \/ \/
		mutable std::recursive_mutex m_controlDataMutex;
		mutable std::condition_variable_any m_controlDataConditionVariable;		// notify_one every time m_controlData is changed

		ControlData m_controlData;
		// /\ /\ /\ /\ /\
		// End of Access only with mutex
		//
	};

}

Q_DECLARE_METATYPE(Sim::SimControlState);
Q_DECLARE_METATYPE(Sim::ControlStatus);


