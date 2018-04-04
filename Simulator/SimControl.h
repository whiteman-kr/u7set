#ifndef SIMCONTROL_H
#define SIMCONTROL_H

#include <set>
#include <atomic>
#include <memory>
#include <QThread>
#include <QtConcurrent>
#include <QMutex>
#include <SimOutput.h>
#include <SimLmModel.h>
#include <SimTimeController.h>

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
			m_lm(lm)
		{
		}

		QFuture<bool> start(std::chrono::microseconds time)
		{
			m_lastStartTime = time;
			m_possibleToAdvanceTo = time;
			m_cylcesCounter ++;
			return m_lm->asyncRunCycle(time);
		}

		QString equipmentId() const
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
		int m_cylcesCounter = 0;
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
	};


	class Control : public QThread, protected Output
	{
		Q_OBJECT

	public:
		explicit Control(Simulator* simualtor, QObject* parent = nullptr);
		virtual ~Control();

	public:
		void reset();

		bool addToRunList(const QString& equipmentId);
		bool addToRunList(const QStringList& equipmentIds);

		void removeFromRunList(const QString& equipmentId);
		void removeFromRunList(const QStringList& equipmentIds);

		bool start(std::chrono::microseconds time = -1us);
		void pause();
		void stop();

		ControlData controlData() const;
		void setControlDataTime();

		SimControlState state() const;
		bool isRunning() const;

		std::chrono::microseconds leftTime() const;

	signals:
		void stateChanged();

	protected:
		virtual void run() override;

		bool processRun();

	private:
		Simulator* m_simualtor = nullptr;

		// Start of access only with mutex
		//
		mutable QMutex m_mutex{QMutex::Recursive};
		ControlData m_controlData;

		// End of Access only with mutex
		//
	};

}

#endif // SIMCONTROL_H
