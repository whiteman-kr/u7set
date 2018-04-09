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

	bool Control::addToRunList(const QString& equipmentId)
	{
		QStringList l;
		l << equipmentId;

		return addToRunList(l);
	}

	bool Control::addToRunList(const QStringList& equipmentIds)
	{
		writeMessage(tr("Add to RunList %1 module(s).").arg(equipmentIds.join(", ")));

		if (state() == SimControlState::Run)
		{
			writeWaning(tr("Adding module to simulation while simulation running will not take effect till simulation is restarted."));
		}

		bool ok = true;
		std::vector<SimControlRunStruct> lms;
		lms.reserve(equipmentIds.size());

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

			lms.emplace_back(lm);
		}

		if (ok == false)
		{
			return false;
		}

		// Add to list
		//
		{
			QMutexLocker l(&m_mutex);

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

		return true;
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

		{
			QMutexLocker l(&m_mutex);

			if (m_controlData.m_lms.empty() == true)
			{
				// Nothing to run
				//
				writeWaning(tr("No selected modules to simulate."));

				m_controlData.m_state = SimControlState::Stop;

				l.unlock();		// Unlock before emitting signal, just in case

				emit stateChanged();
				return false;
			}

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
				assert(false);
				break;

			case SimControlState::Pause:
				m_controlData.m_state = SimControlState::Run;
				m_controlData.m_duration = duration;
				if (m_controlData.m_currentTime >= m_controlData.m_startTime + m_controlData.m_duration)
				{
					m_controlData.m_state = SimControlState::Stop;
				}
				break;

			default:
				assert(false);
			}

			l.unlock();		// Unlock before emitting signal, just in case

			emit stateChanged();
		}

		return true;
	}

	void Control::pause()
	{
		std::chrono::microseconds leftTime{0};

		{
			QMutexLocker l(&m_mutex);
			m_controlData.m_state = SimControlState::Pause;

			leftTime = (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
		}

		emit stateChanged();

		writeMessage(tr("Pause, left time %1, us").arg(leftTime.count()));
		return;
	}

	void Control::stop()
	{
		std::chrono::microseconds leftTime{0};

		{
			QMutexLocker l(&m_mutex);
			m_controlData.m_state = SimControlState::Stop;

			leftTime = (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
		}

		emit stateChanged();

		writeMessage(tr("Stop, left cycle %1").arg(leftTime.count()));
		return;
	}

	ControlData Control::controlData() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData;
	}

	void Control::updateControlDataTime(std::chrono::microseconds currentTime)
	{
		QMutexLocker l(&m_mutex);
		m_controlData.m_currentTime = currentTime;
		return;
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

	std::chrono::microseconds Control::duration() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData.m_duration;
	}

	std::chrono::microseconds Control::leftTime() const
	{
		QMutexLocker l(&m_mutex);
		return (m_controlData.m_startTime + m_controlData.m_duration) - m_controlData.m_currentTime;
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
				// processRun() blocks until state() is changed or time expired
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
		std::vector<SimControlRunStruct>& lms = cd.m_lms;

		if (lms.empty() == true)
		{
			assert(lms.empty() == false);
			writeError(tr("processRun, No LogicModules to simulate."));
			return false;
		}

		for (const SimControlRunStruct& lm : lms)
		{
			auto simLm = m_simualtor->logicModule(lm.equipmentId());

			if (simLm == nullptr)
			{
				assert(simLm);
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
		std::map<QString, QFuture<bool>> tasks;				// Key is LM's equipmentId

		QTime perfmanceTimer;
		perfmanceTimer.start();

		std::chrono::microseconds perfmonaceStartedAt = cd.m_currentTime;

		auto finishTime = cd.m_startTime + cd.m_duration;
		do
		{
			if (isInterruptionRequested() == true)
			{
				break;
			}

			for (SimControlRunStruct& lm : lms)
			{
				auto taskIt = tasks.find(lm.equipmentId());

				if (taskIt != tasks.end())
				{
					QFuture<bool>& f = taskIt->second;

					if (f.isFinished() == true)
					{
						lm.m_possibleToAdvanceTo = lm.m_lastStartTime + lm->cycleDuration();
						tasks.erase(taskIt);

						//qDebug() << "Finished LM " << lm.equipmentId() << " , count = " << lm.m_cylcesCounter;
						continue;	// To for (LM& lm : lms)
					}
					else
					{
						// This task has not been finished yet
						//
						continue;
					}
				}
				else
				{
					// Task not found for this LM
					//
					if (lm.m_possibleToAdvanceTo <= cd.m_currentTime)
					{
						// Task can be start again
						//
						tasks[lm.equipmentId()] = lm.start(cd.m_currentTime);
						continue;
					}

					continue;	// To for (LM& lm : lms)
				}

				assert(false);
			}	// for (SimControlRunStruct& lm : lms)

			std::chrono::microseconds minPossibleTime = lms.front().m_possibleToAdvanceTo;
			for (auto& lm : lms)
			{
				std::chrono::microseconds lmpt = lm.m_possibleToAdvanceTo;
				minPossibleTime = qMin(minPossibleTime, lmpt);
			}

			if (minPossibleTime > cd.m_currentTime)
			{
				auto ahead = minPossibleTime -
							 std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
							// Eto p@#$c tovarischi, c++
							//

				if (ahead > 0us)
				{
					QThread::usleep(ahead.count());
				}

				// Assign new currentTime
				//
				cd.m_currentTime = minPossibleTime;

				QDateTime t = cd.currentTime();
				qDebug() << "CurrentTime changed to: " << t.toString("dd/MM/yyyy HH:mm:ss:zzz");
			}

			QThread::yieldCurrentThread();	// Give some time for tasks
		}
		while (cd.m_duration < 0us ||			// Run always till state is triggered to STOP or PAUSE
			   cd.m_duration != 0us ||			// Run one cycle only (==0)
			   (cd.m_duration > 0us && cd.m_currentTime < finishTime));

		// Wait everything to finish
		//
		for (auto [id, future] : tasks)
		{
			Q_UNUSED(id);
			future.waitForFinished();
		}

		// Update time in m_controlData
		//
		updateControlDataTime(cd.m_currentTime);

		// Some debug info
		//
		std::chrono::microseconds elapsedUsecs{perfmanceTimer.elapsed() * 1000};

		std::chrono::microseconds perfmonaceFinishedAt = cd.m_currentTime;
		std::chrono::microseconds simulatedDiff = perfmonaceFinishedAt - perfmonaceStartedAt;

		double perfRation = static_cast<double>(elapsedUsecs.count()) /
							static_cast<double>(simulatedDiff.count());

		QString logMessage = tr("Simulation time for %1ms, is %2ms physical time, ratio is %3")
								.arg(simulatedDiff.count() / 1000)
								.arg(elapsedUsecs.count() / 1000)
								.arg(perfRation);

		qDebug() << logMessage;
		writeMessage(logMessage);

		return result;
	}

}
