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

				if (presentIt != m_controlData.m_lms.end())
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

	bool Control::start(std::chrono::microseconds time /* = std::chrono::microseconds{-1}*/)
	{
		writeMessage(tr("Start, %1 usecs").arg(time.count()));

		{
			QMutexLocker l(&m_mutex);

			if (m_controlData.m_lms.empty() == true)
			{
				// Nothing to run
				//
				writeWaning(tr("No selected modules to simulate."));

				if (m_controlData.m_state == SimControlState::Stop)
				{
					auto now = std::chrono::system_clock::now();
					m_controlData.m_startTime = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
					m_controlData.m_currentTime = m_controlData.m_startTime;
				}

				m_controlData.m_state = SimControlState::Stop;
				m_controlData.m_leftTime = 0us;

				emit stateChanged();
				return false;
			}

			m_controlData.m_state = SimControlState::Run;
			m_controlData.m_leftTime = time;

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
			leftTime = m_controlData.m_leftTime;
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
			leftTime = m_controlData.m_leftTime;
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

	void Control::setControlDataTime(const ControlData& cd)
	{

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

	std::chrono::microseconds Control::leftTime() const
	{
		QMutexLocker l(&m_mutex);
		return m_controlData.m_leftTime;
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
		// auto startTime =
		// cd.m_currentTime;
		// cd.m_startTime;
		auto finishTime = cd.m_startTime + cd.m_duration;

		std::map<QString, QFuture<bool>> tasks;				// Key is LM's equipmentId

		// Only no  time limit implementation for now (cd.m_leftTime < 0)
		//

		// If it's the first run then run all lm's one time
		//
		if (cd.m_startTime == cd.m_currentTime)
		{
			assert(tasks.empty() == true);

			for (SimControlRunStruct& crs : lms)
			{
				tasks[crs.equipmentId()] = crs->asyncRunCycle(cd.m_currentTime);
			}
		}

		do while here

		while (cd.m_duration < 0us ||			// Run always till state is triggered to STOP or PAUSE
			   cd.m_duration == 0us ||			// Run one cycle
			   (cd.m_duration > 0us && cd.m_currentTime < finishTime))
		{
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

						qDebug() << "Finised LM " << lm.equipmentId() << " , count = " << lm.m_cylcesCounter;
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
			}

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

				cd.m_currentTime = minPossibleTime;
			}

			//--
			//
			if (state() != Run ||
				cd.m_duration == 0us) // Run one cycle
			{
				break;
			}
		}

		// Wait everything to finish
		//
		for (auto [id, future] : tasks)
		{
			Q_UNUSED(id);
			future.waitForFinished();
		}

		auto stopTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
		auto diff = stopTime - cd.m_startTime;
		qDebug() << "Simulation time for 1 sec is " << diff.count() / 1000;

		// Set times to ControlData in this class, as we have a copy
		//
		setControlDataTime(...);

		return result;
	}


}
