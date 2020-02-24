#include "SimControl.h"
#include "Simulator.h"

namespace Sim
{

	Control::Control(Simulator* simualtor, QObject* parent) :
		QThread(parent),
		Output("Controller"),
		m_simulator(simualtor)
	{
		Q_ASSERT(m_simulator);

		QThread::start();

		return;
	}

	Control::~Control()
	{
		stopThread();
		return;
	}

	void Control::stopThread()
	{
		requestInterruption();

		if (bool ok = wait(10000);
			ok == false)
		{
			writeError("Thread forced to terminate.");
			setTerminationEnabled(true);
			terminate();
		}
	}

	void Control::reset()
	{
		writeMessage(tr("Reset"));

		QWriteLocker wl(&m_controlDataLock);
		m_controlData = ControlData{};

		return;
	}

	bool Control::addToRunList(const QString& equipmentId)
	{
		QStringList l;
		l << equipmentId;

		return addToRunList(l);
	}

	int Control::addToRunList(const QStringList& equipmentIds)
	{
		writeMessage(tr("Add to RunList %1 module(s).").arg(equipmentIds.join(", ")));

		if (state() == SimControlState::Run)
		{
			writeWaning(tr("Adding module to simulation while simulation running will not take effect till simulation is restarted."));
		}

		int addedModuleCount = 0;
		std::vector<SimControlRunStruct> lms;
		lms.reserve(equipmentIds.size());

		for (QString id : equipmentIds)
		{
			std::shared_ptr<LogicModule> lm = m_simulator->logicModule(id);
			if (lm == nullptr)
			{
				writeError(QString("Module %1 not found or it does not have simultion ability.").arg(id));
				continue;
			}

			lm->setOverrideSignals(&m_simulator->overrideSignals());

			lms.emplace_back(lm);
			addedModuleCount ++;
		}

		// Add to list
		//
		{
			QWriteLocker wl(&m_controlDataLock);

			for (SimControlRunStruct& scrs : lms)
			{
				// Check if such lm already present in simulation list
				//
				auto presentIt = std::find_if(m_controlData.m_lms.begin(), m_controlData.m_lms.end(),
				[&scrs](auto& lm)
				{
					return lm.m_lm->equipmentId() == scrs.m_lm->equipmentId();
				});

				if (presentIt == m_controlData.m_lms.end())
				{
					m_controlData.m_lms.push_back(std::move(scrs));
				}
			}
		}

		return addedModuleCount;
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

		QWriteLocker wl(&m_controlDataLock);

		for (QString id : equipmentIds)
		{
			m_controlData.m_lms.erase(
						std::remove_if(m_controlData.m_lms.begin(),
									   m_controlData.m_lms.end(),
									   [&id](auto& lm)	{ return lm.equipmentId() == id; }),
						m_controlData.m_lms.end());
		}

		return;
	}

	bool Control::startSimulation(std::chrono::microseconds duration /* = std::chrono::microseconds{-1}*/)
	{
		writeMessage(tr("Start"));

		QWriteLocker wl(&m_controlDataLock);

		if (m_controlData.m_lms.empty() == true)
		{
			// Nothing to run
			//
			writeWaning(tr("No selected modules to simulate."));

			m_controlData.m_state = SimControlState::Stop;

			wl.unlock();		// Unlock before emitting signal, just in case

			emit stateChanged(SimControlState::Stop);
			return false;
		}

		SimControlState state;

		switch (m_controlData.m_state)
		{
		case SimControlState::Stop:
			for (SimControlRunStruct& cs : m_controlData.m_lms)
			{
				cs.m_lastStartTime = 0us;	// it will make LM to reset() before running cycle
			}

			m_controlData.m_state = SimControlState::Run;
			m_controlData.m_startTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
			m_controlData.m_currentTime = m_controlData.m_startTime;
			m_controlData.m_duration = duration;
			break;

		case SimControlState::Run:
			Q_ASSERT(false);
			break;

		case SimControlState::Pause:
			m_controlData.m_state = SimControlState::Run;
			m_controlData.m_duration = duration;
			if (duration != std::chrono::microseconds{-1} &&
				m_controlData.m_currentTime >= m_controlData.m_startTime + m_controlData.m_duration)
			{
				m_controlData.m_state = SimControlState::Stop;
			}
			break;

		default:
			assert(false);
		}

		state = m_controlData.m_state;

		wl.unlock();		// Unlock before emitting signal, just in case

		emit stateChanged(state);

		return true;
	}

	void Control::pause()
	{
		std::chrono::microseconds leftTime{0};

		{
			QWriteLocker wl(&m_controlDataLock);
			m_controlData.m_state = SimControlState::Pause;

			leftTime = (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
		}

		emit stateChanged(SimControlState::Pause);

		writeMessage(tr("Pause, left time %1, us").arg(leftTime.count()));
		return;
	}

	void Control::stop()
	{
		std::chrono::microseconds leftTime{0};

		{
			QWriteLocker wl(&m_controlDataLock);
			m_controlData.m_state = SimControlState::Stop;

			leftTime = (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
		}

		emit stateChanged(SimControlState::Stop);

		writeMessage(tr("Stop, left cycle %1").arg(leftTime.count()));
		return;
	}

	ControlData Control::controlData() const
	{
		QReadLocker rl(&m_controlDataLock);
		return m_controlData;
	}

	void Control::updateControlData(const ControlData& cd)
	{
		QWriteLocker wl(&m_controlDataLock);

		m_controlData.m_currentTime = cd.m_currentTime;

		for (SimControlRunStruct& rs : m_controlData.m_lms)
		{
			for (const SimControlRunStruct& cdrs : cd.m_lms)
			{
				if (cdrs.equipmentId() == rs.equipmentId())
				{
					rs.m_lastStartTime = cdrs.m_lastStartTime;
					break;
				}
			}
		}

		return;
	}

	SimControlState Control::state() const
	{
		QReadLocker rl(&m_controlDataLock);
		return m_controlData.m_state;
	}

	bool Control::isRunning() const
	{
		QReadLocker rl(&m_controlDataLock);
		return m_controlData.m_state == SimControlState::Run;
	}

	std::chrono::microseconds Control::duration() const
	{
		QReadLocker rl(&m_controlDataLock);
		return m_controlData.m_duration;
	}

	std::chrono::microseconds Control::leftTime() const
	{
		QReadLocker rl(&m_controlDataLock);
		return (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
	}

	void Control::run()
	{
		if (m_simulator == nullptr)
		{
			Q_ASSERT(m_simulator);
			return;
		}

		while (isInterruptionRequested() == false)
		{
			switch (state())
			{
			case SimControlState::Stop:
				// Have some rest
				//
				msleep(200);
				break;

			case SimControlState::Run:
				// !!! processRun() blocks until state() is changed or time expired
				//
				if (bool ok = processRun();	// Blocks here
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
				Q_ASSERT(false);
				return;
			}

		} // while

		return;
	}


	bool Control::processRun()
	{
		using namespace std::chrono;

		bool result = true;
		ControlData cd = controlData();

		// Get simulation LogicModules
		//
		std::vector<SimControlRunStruct>& lms = cd.m_lms;

		if (lms.empty() == true)
		{
			assert(lms.empty() == false);
			writeError(tr("processRun, No LogicModules to simulate."));
			return false;
		}

		for (const SimControlRunStruct& lm : lms)
		{
			auto simLm = m_simulator->logicModule(lm.equipmentId());

			if (simLm == nullptr)
			{
				Q_ASSERT(simLm);
				writeError(tr("processRun, LogicModule %1 not found").arg(lm.equipmentId()));
				result = false;
				continue;
			}
		}

		if (result == false)
		{
			return false;
		}

		// --
		//
		QTime perfmanceTimer;
		perfmanceTimer.start();

		microseconds perfmonaceStartedAt = cd.m_currentTime;

		auto finishTime = cd.m_startTime + cd.m_duration;
		do
		{
			if (isInterruptionRequested() == true)
			{
				break;
			}

			for (SimControlRunStruct& lm : lms)
			{
				if (lm.m_task.has_value() == true)
				{
					QFuture<bool>& f = lm.m_task.value();

					if (f.isFinished() == true)
					{
						lm.m_possibleToAdvanceTo = lm.m_lastStartTime + lm->cycleDuration();
						lm.m_task.reset();

						lm.afterWorkCycleTask(m_simulator->appSignalManager());

						//qDebug() << "Finished LM " << lm.equipmentId() << " , count = " << lm.m_cylcesCounter;
					}
					else
					{
						// This task has not been finished yet
						//
					}
				}
				else
				{
					// Task not found for this LM
					//
					if (lm.m_possibleToAdvanceTo <= cd.m_currentTime)
					{
						// Task can be STARTED again
						//
						lm.m_task = lm.start(cd.m_currentTime);
					}
				}
			}	// for (SimControlRunStruct& lm : lms)

			// Calculate minimum possible time
			//
			microseconds minPossibleTime = lms.front().m_possibleToAdvanceTo;
			for (const SimControlRunStruct& lm : lms)
			{
				microseconds lmpt = lm.m_possibleToAdvanceTo;

				if (lmpt.count() < minPossibleTime.count())	// using count() makes it faster, no need to convert to different ratio
				{
					minPossibleTime = lmpt;
				}
			}

			// Shift current time if requored
			//
			if (minPossibleTime > cd.m_currentTime)
			{
				// If current simulation is ahead of physical time, pause it a little bit
				//
				if (auto ahead = minPossibleTime - duration_cast<microseconds>(system_clock::now().time_since_epoch());
					ahead > 0us)
				{
					QThread::usleep(ahead.count());
				}

				// Assign new currentTime
				//
				cd.m_currentTime = minPossibleTime;

//				QDateTime t = cd.currentTime();
//				qDebug() << "CurrentTime changed to: " << t.toString("dd/MM/yyyy HH:mm:ss:zzz");
			}

			if (state() != SimControlState::Run)
			{
				break;		// Usually exit point from do-while loop
			}

			// QThread::yieldCurrentThread();	// Give some time for tasks
			// This code is instead of QThread::yieldCurrentThread,
			// It's not tested for several LMs, subject to examine perfomnace
			//
			for (SimControlRunStruct& lm : lms)
			{
				if (lm.m_task.has_value() == true)
				{
					lm.m_task->waitForFinished();
					break;
				}
			}
		}
		while (cd.m_duration < 0us ||			// Run always till state is triggered to STOP or PAUSE
			   cd.m_duration != 0us ||			// Run one cycle only (==0)
			   (cd.m_duration > 0us && cd.m_currentTime < finishTime));

		// Wait everything to finish
		//
		for (SimControlRunStruct& lm : lms)
		{
			if (lm.m_task.has_value() == true)
			{
				QFuture<bool>& future = lm.m_task.value();

				future.waitForFinished();

				// Perform post run cycle actions
				//
				lm.afterWorkCycleTask(m_simulator->appSignalManager());
			}
		}

		// Update current time and last time in m_controlData
		//
		updateControlData(cd);

		// Some debug info
		//
		microseconds elapsedUsecs{perfmanceTimer.elapsed() * 1000};

		microseconds perfmonaceFinishedAt = cd.m_currentTime;
		microseconds simulatedDiff = perfmonaceFinishedAt - perfmonaceStartedAt;

		double perfRation = static_cast<double>(simulatedDiff.count()) /
							static_cast<double>(elapsedUsecs.count());

		QString logMessage = tr("Simulation time for %1ms, is %2ms physical time, ratio is %3")
								.arg(simulatedDiff.count() / 1000)
								.arg(elapsedUsecs.count() / 1000)
								.arg(perfRation);

		//qDebug() << logMessage;
		writeMessage(logMessage);

		return result;
	}

}
