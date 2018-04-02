#include "SimControl.h"
#include "Simulator.h"

namespace Sim
{

	Control::Control(Simulator* simualtor, QObject* parent) :
		QThread(parent),
		Output("Controller"),
		m_simualtor(simualtor)
	{
		assert(m_simualtor);

		QThread::start();

		return;
	}

	Control::~Control()
	{
		requestInterruption();

		if (bool ok = wait(10000);
			ok == false)
		{
			writeError("Thread forced to terminate.");
			setTerminationEnabled(true);
			terminate();
		}

		return;
	}

	void Control::reset()
	{
		writeMessage(tr("Reset"));

		QMutexLocker l(&m_mutex);
		m_controlData = ControlData();

		return;
	}

	void Control::addToRunList(const QString& equipmentId)
	{
		QStringList l;
		l << equipmentId;

		return addToRunList(l);
	}

	void Control::addToRunList(const QStringList& equipmentIds)
	{
		writeMessage(tr("Add to RunList %1 module(s).").arg(equipmentIds.join(", ")));

		bool ok = true;

		for (QString id : equipmentIds)
		{
			std::shared_ptr<LogicModule> lm = m_simualtor->logicModule(id);
			if (lm == nullptr)
			{
				writeError(QString("Module %1 not found.").arg(id));
				assert(lm);
				ok = false;
				continue;
			}
		}

		if (ok == false)
		{
			return;
		}

		// Add to list
		//
		QMutexLocker l(&m_mutex);

		for (QString id : equipmentIds)
		{
			m_controlData.m_equipmentIds.insert(id);
		}

		return;
	}

	void Control::removeFromRunList(const QString& equipmentId)
	{
		QStringList l;
		l << equipmentId;

		return removeFromRunList(l);
	}

	void Control::removeFromRunList(const QStringList& equipmentIds)
	{
		writeMessage(tr("Remove from RunList %1 module(s).").arg(equipmentIds.join(", ")));

		QMutexLocker l(&m_mutex);

		for (QString id : equipmentIds)
		{
			m_controlData.m_equipmentIds.erase(id);
		}

		return;
	}

	bool Control::start(int cycles /*= -1*/)
	{
		writeMessage(tr("Start %1 cylces").arg(cycles));

		m_timeController.reset();

		{
			QMutexLocker l(&m_mutex);

			if (m_controlData.m_equipmentIds.empty() == true)
			{
				// Nothing to run
				//
				writeWaning(tr("No selected modules to simulate."));

				m_controlData.m_state = SimControlState::Stop;
				m_controlData.m_leftCycles = 0;

				emit stateChanged();
				return false;
			}

			m_controlData.m_state = SimControlState::Run;
			m_controlData.m_leftCycles = cycles;

			emit stateChanged();
		}

		return true;
	}

	void Control::pause()
	{
		int leftCycles = 0;

		{
			QMutexLocker l(&m_mutex);
			m_controlData.m_state = SimControlState::Pause;
			leftCycles = m_controlData.m_leftCycles;
		}

		emit stateChanged();

		writeMessage(tr("Pause, left cycle %1").arg(leftCycles));
		return;
	}

	void Control::stop()
	{
		int leftCycles = 0;

		{
			QMutexLocker l(&m_mutex);
			m_controlData.m_state = SimControlState::Stop;
			leftCycles = m_controlData.m_leftCycles;
		}

		emit stateChanged();

		writeMessage(tr("Stop, left cycle %1").arg(leftCycles));
		return;
	}

	ControlData Control::controlData() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData;
	}

	SimControlState Control::state() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData.m_state;
	}

	bool Control::isRunning() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData.m_state == SimControlState::Run;
	}

	int Control::leftCycles() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData.m_leftCycles;
	}

	void Control::run()
	{
		if (m_simualtor == nullptr)
		{
			assert(m_simualtor);
			return;
		}

		while (isInterruptionRequested() == false)
		{
			switch (state())
			{
			case SimControlState::Stop:
				// Have some rest
				//
				int If_required_reset_all_LMs_here_questionmark;	//?

				msleep(200);
				break;

			case SimControlState::Run:
				// 1. Check if all LogicModules have reached their cycles limit.
				// 2. If smthng is not running but has cycles, start it
				//
				if (bool ok = processRun();
					ok == false)
				{
					// Some error in simulation, stop the simulation
					//
					reset();
				}
				break;

			case SimControlState::Pause:
				// Have some rest
				//

				msleep(200);
				break;

			default:
				assert(false);
				return;
			}

		} // while

		return;
	}

	bool Control::processRun()
	{
		bool result = true;
		ControlData cd = controlData();

		// Get simulation LogicModules
		//
		std::vector<std::shared_ptr<LogicModule>> lms;
		lms.reserve(cd.m_equipmentIds.size());

		for (QString id : cd.m_equipmentIds)
		{
			auto lm = m_simualtor->logicModule(id);

			if (lm == nullptr)
			{
				assert(lm);
				writeError(tr("processRun, LogicModuel %1 not found").arg(id));
				result = false;
				continue;
			}

			lms.push_back(lm);
		}

		if (lms.empty() == true)
		{
			assert(lms.empty() == false);
			writeError(tr("processRun, No LogicMoudles to simulate."));
			result = false;
			return result;
		}

		// Check previous tasks
		//
		for (auto it = m_lmTasks.begin(); it != m_lmTasks.end(); ++it)
		{
			const QString& lmId = it->first;
			QFuture<bool>& task = it->second;


		}

		// Detect which modules must run now, depending on work cycle duariond and TimeController
		//
//		auto ct = m_timeController.currentTime();
//		if (ct > std::chrono::system_clock::now())
//		{
//			// Current simulation process is ahead of real time, skip this cycle
//			//
//			return true;
//		}


		// Run 1 cycle of simulation
		//
		//device.processOperate()

		// Set new value to TimeController
		//

		return result;
	}


}
