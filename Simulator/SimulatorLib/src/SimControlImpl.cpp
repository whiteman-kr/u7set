#include "SimControlImpl.h"
#include "SimLogicModuleImpl.h"
#include "SimulatorPrivate.h"


namespace Sim
{
	SimControlRunStruct::SimControlRunStruct(std::shared_ptr<LogicModuleImpl> lm) :
		m_lm(std::move(lm))
	{
	}

	QFuture<bool> SimControlRunStruct::start(std::chrono::microseconds time,
											 const QDateTime& currentDateTime,
											 std::condition_variable& cvFinished)
	{
		bool reset = m_lastStartTime == 0us;

		m_lastStartTime = time;
		m_possibleToAdvanceTo = time;
		m_cyclesCounter++;
		return m_lm->asyncRunCycle(time, currentDateTime, m_cyclesCounter, reset, cvFinished);
	}

	const QString& SimControlRunStruct::equipmentId() const
	{
		return m_lm->equipmentId();
	}

	LogicModuleImpl* SimControlRunStruct::operator->()
	{
		return m_lm.get();
	}

	const LogicModuleImpl* SimControlRunStruct::operator->() const
	{
		return m_lm.get();
	}

	//
	//
	// Sim::ControlImpl
	//
	//
	ControlImpl::ControlImpl(SimulatorPrivate* simualtor, QObject* parent) :
		QThread(parent),
		m_simulator(simualtor),
		m_log(simualtor->log(), "Controller")
	{
		Q_ASSERT(m_simulator);

		QThread::start();

		this->moveToThread(this);

		return;
	}

	ControlImpl::~ControlImpl()
	{
		stopThread();
		return;
	}

	void ControlImpl::stopThread()
	{
		requestInterruption();

		if (bool ok = wait(10000); ok == false)
		{
			m_log.writeError("Thread forced to terminate.");
			setTerminationEnabled(true);
			terminate();
		}
	}

	void ControlImpl::reset()
	{
		m_log.writeDebug(tr("Reset"));

		{
			std::lock_guard locker(m_controlDataMutex);
			m_controlData = ControlData{};
		}

		m_controlDataConditionVariable.notify_one();

		// Wait when simulation thread exit from simulation loop (Sim::ControlImpl::processRun).
		//
		m_insideProcessRun.wait(true);

		m_simulator->software().stopSimulation();

		return;
	}


	int ControlImpl::setRunList(QStringList equipmentIds)
	{
		// if equipmentIds is empty then add all modules to simulation
		//
		{
			auto lms = m_simulator->logicModules();

			for (const auto& lm : lms)
			{
				equipmentIds << lm->equipmentId();
			}

			if (equipmentIds.isEmpty() == true)
			{
				m_log.writeWarning(tr("Nothing to simulate, no LogicModules are found."));
				return 0;
			}
		}

		// --
		//
		m_log.writeDebug(tr("Add to RunList %1 module(s).").arg(equipmentIds.join(", ")));

		if (state() == SimControlState::Run)
		{
			m_log.writeWarning(
				tr("Adding module to simulation while simulation running will not take effect till simulation is restarted."));
		}

		int addedModuleCount = 0;
		std::vector<SimControlRunStruct> lms;
		lms.reserve(equipmentIds.size());

		for (QString id : equipmentIds)
		{
			std::shared_ptr<LogicModuleImpl> lm = m_simulator->logicModule(id);
			if (lm == nullptr)
			{
				m_log.writeError(QString("Module %1 not found or it does not have simultion ability.").arg(id));
				continue;
			}

			lms.emplace_back(lm);
			addedModuleCount++;
		}

		// set list
		//
		{
			std::lock_guard locker(m_controlDataMutex);

			// Add new LMs, keep old
			//
			for (SimControlRunStruct& scrs : lms)
			{
				// Check if such lm already present in simulation list
				//
				auto presentIt = std::find_if(m_controlData.m_lms.begin(),
											  m_controlData.m_lms.end(),
											  [&scrs](auto& lm)
											  {
												  return lm.m_lm->equipmentId() == scrs.m_lm->equipmentId();
											  });

				if (presentIt == m_controlData.m_lms.end())
				{
					m_controlData.m_lms.push_back(scrs);
				}
			}

			// Remove LMs
			//
			m_controlData.m_lms.erase(std::remove_if(m_controlData.m_lms.begin(),
													 m_controlData.m_lms.end(),
													 [&lms](SimControlRunStruct& lm) -> bool
													 {
														 QString id = lm.equipmentId();
														 return find_if(lms.begin(),
																		lms.end(),
																		[&id](SimControlRunStruct& lm)
																		{
																			return lm.equipmentId() == id;
																		}) == lms.end();
													 }),
									  m_controlData.m_lms.end());
		}

		m_controlDataConditionVariable.notify_one();

		return addedModuleCount;
	}

	void ControlImpl::removeFromRunList(const QString& equipmentId)
	{
		QStringList l;
		l << equipmentId;

		return removeFromRunList(l);
	}

	void ControlImpl::removeFromRunList(const QStringList& equipmentIds)
	{
		m_log.writeDebug(tr("Remove from RunList %1 module(s).").arg(equipmentIds.join(", ")));

		std::lock_guard locker(m_controlDataMutex);

		for (QString id : equipmentIds)
		{
			m_controlData.m_lms.erase(std::remove_if(m_controlData.m_lms.begin(),
													 m_controlData.m_lms.end(),
													 [&id](auto& lm)
													 {
														 return lm.equipmentId() == id;
													 }),
									  m_controlData.m_lms.end());
		}

		m_controlDataConditionVariable.notify_one();

		return;
	}

	bool ControlImpl::startSimulation(std::chrono::microseconds duration /* = std::chrono::microseconds{-1}*/)
	{
		using namespace std::chrono;

		m_log.writeDebug(tr("Start, duration microseconds: %1").arg(duration.count()));

		std::unique_lock locker(m_controlDataMutex);

		if (m_controlData.m_lms.empty() == true)
		{
			// Nothing to run
			//
			m_log.writeWarning(tr("No selected modules to simulate."));

			m_controlData.m_state = SimControlState::Stop;

			ControlStatus cs{m_controlData};

			locker.unlock(); // Unlock before emitting signal
			m_controlDataConditionVariable.notify_one();

			emit stateChanged(cs.m_state);
			emit statusUpdate(cs);

			return false;
		}

		switch (m_controlData.m_state)
		{
		case SimControlState::Stop:
			{
				m_controlData.m_state = SimControlState::Run;

				m_simulator->software().startSimulation(m_simulator->currentProfileName());

				m_controlData.m_startTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());

				// It will make start time on the edge of 5ms, it will make nice timestamp
				//
				m_controlData.m_startTime = (m_controlData.m_startTime / 5000) * 5000;

				m_controlData.m_sliceStartTime = m_controlData.m_startTime;
				m_controlData.m_currentTime = m_controlData.m_sliceStartTime;
				m_controlData.m_duration = duration;

				int offsetFromUtcMs = QDateTime::currentDateTime().offsetFromUtc() * 1000;

				for (SimControlRunStruct& cs : m_controlData.m_lms)
				{
					cs.m_lastStartTime = 0us; // it will make LM to reset() before running cycle
					m_simulator->overrideSignals().requestToResetOverrideScripts(
						cs.equipmentId());    // It will reset all scripts, clear global variables, etc

					// It sets nonvalid point to realtime trends
					//
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_controlData.m_currentTime);

					TimeStamp plantTime{ms.count() + offsetFromUtcMs};
					TimeStamp localTime{plantTime};
					TimeStamp systemTime{ms.count()};

					m_simulator->appSignalManager().setData(cs.equipmentId(), {}, plantTime, localTime, systemTime);
				}
			}
			break;

		case SimControlState::Run:
			Q_ASSERT(false);
			break;

		case SimControlState::Pause:
			m_controlData.m_state = SimControlState::Run;
			m_controlData.m_sliceStartTime = m_controlData.m_currentTime;
			m_controlData.m_duration = duration;
			break;

		default:
			assert(false);
		}

		ControlStatus cs{m_controlData};

		locker.unlock(); // Unlock before emitting signal

		m_controlDataConditionVariable.notify_one();

		emit stateChanged(cs.m_state);
		emit statusUpdate(cs);

		return true;
	}

	void ControlImpl::pause()
	{
		std::chrono::microseconds leftTime{0};
		ControlStatus cs;

		{
			std::lock_guard locker(m_controlDataMutex);
			m_controlData.m_state = SimControlState::Pause;

			leftTime = (m_controlData.m_sliceStartTime + m_controlData.m_duration) - m_controlData.m_currentTime;
			cs = ControlStatus{m_controlData};
		}

		m_controlDataConditionVariable.notify_one();

		emit stateChanged(cs.m_state);
		emit statusUpdate(cs);

		m_log.writeDebug(tr("Pause, left time %1, us").arg(leftTime.count()));
		return;
	}

	void ControlImpl::stop()
	{
		std::chrono::microseconds leftTime{0};

		ControlStatus cs;
		{
			std::lock_guard locker(m_controlDataMutex);
			m_controlData.m_state = SimControlState::Stop;

			leftTime = (m_controlData.m_sliceStartTime + m_controlData.m_duration) - m_controlData.m_currentTime;
			cs = ControlStatus{m_controlData};
		}

		m_controlDataConditionVariable.notify_one();

		// Wait when simulation thread exit form simulation loop (Sim::ControlImpl::processRun).
		//
		m_insideProcessRun.wait(true);

		m_simulator->software().stopSimulation();

		emit stateChanged(cs.m_state);
		emit statusUpdate(cs);

		m_log.writeDebug(tr("Stop, left cycle %1").arg(leftTime.count()));
		return;
	}

	ControlData ControlImpl::controlData() const
	{
		std::lock_guard locker(m_controlDataMutex);
		return m_controlData;
	}

	void ControlImpl::updateControlData(const ControlData& cd)
	{
		std::lock_guard locker(m_controlDataMutex);

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

		m_controlDataConditionVariable.notify_one();

		return;
	}

	SimControlState ControlImpl::state() const
	{
		std::lock_guard locker(m_controlDataMutex);
		return m_controlData.m_state;
	}

	bool ControlImpl::isRunning() const
	{
		std::lock_guard locker(m_controlDataMutex);
		return m_controlData.m_state == SimControlState::Run;
	}

	std::chrono::microseconds ControlImpl::duration() const
	{
		std::lock_guard locker(m_controlDataMutex);
		return m_controlData.m_duration;
	}

	std::chrono::microseconds ControlImpl::leftTime() const
	{
		std::lock_guard locker(m_controlDataMutex);
		return (m_controlData.m_sliceStartTime + m_controlData.m_duration) - m_controlData.m_currentTime;
	}

	double ControlImpl::speedFactor() const
	{
		return m_speedFactor.load(std::memory_order_relaxed);
	}

	void ControlImpl::setSpeedFactor(double value)
	{
		value = std::clamp(value, 0.1, 256.0);
		m_speedFactor.store(value, std::memory_order_relaxed);

		m_log.writeText(tr("Speed factor set to %1").arg(value));
	}

	void ControlImpl::run()
	{
		if (m_simulator == nullptr)
		{
			Q_ASSERT(m_simulator);
			return;
		}

		while (isInterruptionRequested() == false)
		{
			SimControlState currentState = state();

			if (currentState == SimControlState::Stop || currentState == SimControlState::Pause)
			{
				// Have some rest
				//
				std::unique_lock locker(m_controlDataMutex);
				m_controlDataConditionVariable.wait_for(locker,
														std::chrono::milliseconds{1000},
														[currentState, this]()
														{
															return m_controlData.m_state != currentState;
														});

				currentState = m_controlData.m_state;
			}

			if (currentState == SimControlState::Run)
			{
				m_insideProcessRun.store(true);

				// !!! processRun() blocks until state() is changed or time expired
				//
				bool ok = processRun(); // Blocks here

				if (ok == false)
				{
					// Some error in simulation, stop the simulation
					//
					reset();
				}

				m_insideProcessRun.store(false);
				m_insideProcessRun.notify_one();
			}
		} // while

		return;
	}


	bool ControlImpl::processRun()
	{
		using namespace std::chrono;

		bool result = true;
		ControlData cd = controlData(); // Initialize local data with actual simulation ControlData

		// Get simulation LogicModules
		//
		std::vector<SimControlRunStruct>& lms = cd.m_lms; // Referense to the !local! variable cd

		if (lms.empty() == true)
		{
			assert(lms.empty() == false);
			m_log.writeError(tr("processRun, No LogicModules to simulate."));
			return false;
		}

		std::chrono::microseconds minimulLmWorkcycle{5000};

		for (const SimControlRunStruct& lm : lms)
		{
			auto simLm = m_simulator->logicModule(lm.equipmentId());

			if (simLm == nullptr)
			{
				Q_ASSERT(simLm);
				m_log.writeError(tr("processRun, LogicModule %1 not found").arg(lm.equipmentId()));
				result = false;
				continue;
			}

			minimulLmWorkcycle =
				std::min(minimulLmWorkcycle, std::chrono::microseconds{simLm->lmDescription().logicUnit().m_cycleDuration});
		}

		if (result == false)
		{
			return false;
		}

		// --
		//
		QElapsedTimer perfmanceTimer;
		perfmanceTimer.start();

		microseconds perfmonaceStartedAt = cd.m_currentTime;
		qint64 timeStatusUpdateCounter = 0;
		QDateTime currentDateTime =
			QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(cd.m_currentTime).count());

		auto finishTime = cd.m_sliceStartTime + cd.m_duration;

		double currentSpeedFactor = speedFactor();

		std::mutex fakeMutex;
		std::condition_variable someLmFinishedSimulation;

		do
		{
			if (isInterruptionRequested() == true)
			{
				break;
			}

			// Get data from fiber optic channels (LM, OCM)
			// No concurrent run is required, perfomance measurements show that in
			// concurent mode it it much slower then this code
			//
			bool allLmsArePoweredOff = true;

			for (SimControlRunStruct& lm : lms)
			{
				if (lm->isPowerOff() == true)
				{
					continue;
				}
				else
				{
					allLmsArePoweredOff = false;
				}

				// Run receiveConnectionsData(...) only for running LMs.
				//
				if (lm.m_task.has_value() == false)
				{
					lm->receiveConnectionsData(cd.m_currentTime);
				}
			}

			// Check if workcylce finished on lms then fetch data
			// Start new workcycle on finished lms
			//
			for (SimControlRunStruct& lm : lms)
			{
				if (lm.m_task.has_value() == true)
				{
					QFuture<bool>& f = lm.m_task.value();

					if (f.isFinished() == true)
					{
						lm.m_possibleToAdvanceTo = lm.m_lastStartTime + lm->cycleDuration();
						lm.m_task.reset();
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
						// Here is m_jsEngine and script must run in the same thread as m_jsEngine belongs
						//
						m_simulator->overrideSignals().runOverrideScripts(lm.equipmentId(), lm.m_cyclesCounter);

						// Task can be STARTED again
						//
						lm.m_task = lm.start(cd.m_currentTime, currentDateTime, someLmFinishedSimulation);
					}
				}
			} // for (SimControlRunStruct& lm : lms)

			// Calculate minimum possible time
			//
			microseconds minPossibleTime{0};
			for (const SimControlRunStruct& lm : lms)
			{
				if (lm.m_lm->isPowerOff() == true)
				{
					continue;
				}

				if (minPossibleTime == 0us) // First init
				{
					minPossibleTime = lm.m_possibleToAdvanceTo;
				}

				if (microseconds lmpt = lm.m_possibleToAdvanceTo; lmpt < minPossibleTime)
				{
					minPossibleTime = lmpt;
				}
			}

			if (minPossibleTime == 0us)
			{
				// All LMs are switched off, but the time still must move forward
				//
				minPossibleTime = duration_cast<microseconds>(system_clock::now().time_since_epoch());
			}

			// Shift current time if required
			//
			if (minPossibleTime > cd.m_currentTime)
			{
				if (currentSpeedFactor != speedFactor())
				{
					currentSpeedFactor = speedFactor();
					perfmanceTimer.restart();
					perfmonaceStartedAt = cd.m_currentTime;
				}

				if (currentSpeedFactor < 128.0)
				{
					// If current simulation is ahead of physical time, pause it a little bit
					//
					microseconds timeEllapsed{perfmanceTimer.elapsed() * static_cast<unsigned long long>(1000.0 * currentSpeedFactor)};
					microseconds simulatedTime = cd.m_currentTime - perfmonaceStartedAt;
					microseconds ahead = simulatedTime - timeEllapsed;

					if (ahead > 5us)
					{
						if (ahead > 100ms) // sleep no more then 100ms.
						{
							ahead = 100ms;
						}

						unsigned long usTimeToSleep = static_cast<unsigned long>(ahead.count());
						QThread::usleep(usTimeToSleep);
					}
				}

				// Assign new currentTime
				//
				cd.m_currentTime = minPossibleTime;
				currentDateTime =
					QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(cd.m_currentTime).count());

				if (std::abs(perfmanceTimer.elapsed() - timeStatusUpdateCounter) >=
					125) // Update every ~125 ms, perfmanceTimer can be restarted,
				{        // so std::abs() was added.
					// Emit this information signal every 125 ms, we don't need to send it every cycle
					//
					timeStatusUpdateCounter = perfmanceTimer.elapsed();
					emit statusUpdate(ControlStatus{cd});
				}
			}
			else
			{
				if (allLmsArePoweredOff == true &&
					std::abs(perfmanceTimer.elapsed() - timeStatusUpdateCounter) >= 125) // Update every ~125 ms
				{
					// Emit this information signal every 100 ms, we don't need to send it every cycle
					//
					timeStatusUpdateCounter = perfmanceTimer.elapsed();
					emit statusUpdate(ControlStatus{cd});
				}
			}

			if (state() != SimControlState::Run)
			{
				break; // Usually exit point from do-while loop
			}

			if (cd.m_duration == 0us || (cd.m_duration > 0us && cd.m_currentTime >= finishTime))
			{
				// Simulation time is time up, set PAUSE mode
				//
				break;
			}

			// Give some time for tasks
			//
			bool hadWait = false;

			for (SimControlRunStruct& lm : lms)
			{
				if (lm.m_task.has_value() == true && lm->isPowerOff() == false)
				{
					// There is at least one lm which is running simulation right now.
					// WAIT for at least shortest workcyle time.
					//
					std::unique_lock fakeLock{fakeMutex};

					//	someLmFinishedSimulation.wait_for(fakeLock, minimulLmWorkcycle, [&lms](){
					//		return std::any_of(lms.begin(), lms.end(), [](SimControlRunStruct& lm)
					//		{
					//			return  lm.m_task.has_value() == true &&
					//					lm.m_task.value().isFinished() == true &&
					//					lm->isPowerOff() == false;
					//		});
					//	});

					someLmFinishedSimulation.wait_for(fakeLock,
													  minimulLmWorkcycle,
													  []()
													  {
														  return true;
													  });

					hadWait = true;
					break; // At least one LM has finished the work
				}
			}

			if (hadWait == true)
			{
				continue;
			}

			// If all lms are switched off then sleep for one workcycle
			//
			allLmsArePoweredOff = true;

			for (SimControlRunStruct& lm : lms)
			{
				if (lm->isPowerOff() == false)
				{
					allLmsArePoweredOff = false;
					break;
				}
			}

			if (allLmsArePoweredOff == true)
			{
				QThread::usleep(static_cast<unsigned long>(minimulLmWorkcycle.count()));
			}
		} while (true); // Run always till state is triggered to STOP or PAUSE

		// Wait everything to finish
		//
		for (SimControlRunStruct& lm : lms)
		{
			if (lm.m_task.has_value() == true)
			{
				QFuture<bool>& future = lm.m_task.value();
				future.waitForFinished();
			}
		}

		// Update current time and last time in m_controlData
		//
		updateControlData(cd);
		emit statusUpdate(ControlStatus{controlData()}); // Don't use cd! As it has not updated m_state

		if (state() == SimControlState::Run)
		{
			pause();
		}

		// Some debug info
		//
		microseconds elapsedUsecs{perfmanceTimer.elapsed() * 1000};

		microseconds perfmonaceFinishedAt = cd.m_currentTime;
		microseconds simulatedDiff = perfmonaceFinishedAt - perfmonaceStartedAt;

		double perfRation = static_cast<double>(simulatedDiff.count()) / static_cast<double>(elapsedUsecs.count());

		QString logMessage = tr("Simulation time for %1ms, is %2ms physical time, ratio is %3")
								 .arg(simulatedDiff.count() / 1000)
								 .arg(elapsedUsecs.count() / 1000)
								 .arg(perfRation);

		//		if (unlockTimer() == true)
		//		{
		//			qDebug() << logMessage;
		//		}
		m_log.writeDebug(logMessage);

		return result;
	}

} // namespace Sim
