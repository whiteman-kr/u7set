#ifndef SIMCONTROL_H
#define SIMCONTROL_H

#include <set>
#include <atomic>
#include <QThread>
#include <QMutex>
#include <SimOutput.h>
#include <SimTimeController.h>

namespace Sim
{
	class Simulator;

	enum class SimControlState
	{
		Stop,
		Run,
		Pause
	};


	struct ControlData
	{
		std::set<QString> m_equipmentIds;

		int m_leftCycles = -1;
		SimControlState m_state = SimControlState::Stop;
	};


	class Control : public QThread, protected Output
	{
		Q_OBJECT

	public:
		explicit Control(Simulator* simualtor, QObject* parent = nullptr);
		virtual ~Control();

	public:
		void reset();
		void addToRunList(const QStringList& equipmentIds);
		void removeFromRunList(const QStringList& equipmentIds);

		bool start(int cycles = -1);
		void pause();
		void stop();

		ControlData controlData() const;

		SimControlState state() const;
		bool isRunning() const;

		int leftCycles() const;

	signals:
		void stateChanged();

	protected:
		virtual void run() override;

		bool processRun();

	private:
		Simulator* m_simualtor = nullptr;
		TimeController m_timeController;

		// Start of access only with mutex
		//
		mutable QMutex m_mutex{QMutex::Recursive};
		ControlData m_controlData;
		// End of Access only with mutex
		//
	};

}

#endif // SIMCONTROL_H
