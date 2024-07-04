#include "TestObserver.h"

namespace TestSuite
{
	TestObserver::TestObserver(ClientLib::ISignalDataServer& signalDataServer,
							   const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
							   ILogFile* logFile) :
		m_signalDataServer{signalDataServer},
		m_softwareInfo{softwareInfo},
		m_appDataServices{appDataServices},
		m_logFile{logFile}
	{
	}

	bool TestObserver::start()
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		m_rtDataProvide = std::make_unique<ClientLib::RtDataProvider>(m_signalDataServer, m_logFile);

		m_rtDataProvide->setSamplePeriod(s_defaultSamplePeriod);
		m_rtDataProvide->createConnections(m_softwareInfo, m_appDataServices);

		// Wait till all AppDataServices are connected.
		//
		bool connected = m_rtDataProvide->allConnected(s_connectionTimeout);

		if (connected == false && m_logFile != nullptr)
		{
			auto stats = m_rtDataProvide->statistics();

			m_logFile->writeError(QObject::tr("RtDataProvider connection timeout, connected %1 of %2 AppDataServices.")
									  .arg(stats.isConnected)
									  .arg(m_rtDataProvide->size()));
			stop();
			return false;
		}

		// Add signal to realtime observation.
		//
		QStringList trendSignals;
		trendSignals.reserve(m_expectations.size());

		for (const auto& e : m_expectations)
		{
			trendSignals.push_back(e->appSignalId);
		}

		m_rtDataProvide->setData(s_defaultSamplePeriod, trendSignals);

		// Fill m_waitPrivateData.
		//
		{
			m_waitPrivateData = {};

			for (auto& e : m_expectations)
			{
				e->operative = {}; // Clear expectation state.
			}
		}

		// Waiting for the first records of states. If we have instant initial condition, than this "wait-for-first-records" allows to
		// have pre initial condition and to calculate how long it took to satisfy all times.
		//
		{
			QObject::connect(m_rtDataProvide.get(), &ClientLib::RtDataProvider::dataReady, this, &TestObserver::dataReady);
			QObject::connect(m_rtDataProvide.get(), &ClientLib::RtDataProvider::requestError, this, &TestObserver::requestError);
			QObject::connect(m_rtDataProvide.get(), &ClientLib::RtDataProvider::connectionLost, this, &TestObserver::connectionLost);

			QEventLoop eventLoop;
			QTimer::singleShot(1000, &eventLoop, &QEventLoop::quit);
			QObject::connect(this, &TestObserver::dataReceived, &eventLoop, &QEventLoop::quit);
			eventLoop.exec();
		}

		return true;
	}

	void TestObserver::stop()
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		m_rtDataProvide.reset();

		return;
	}

	void TestObserver::clear()
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		stop();
		m_expectations.clear();

		return;
	}

	bool TestObserver::wait(int timeoutMs)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		return waitPrivate(m_initialExpectationId, timeoutMs);
	}

	void TestObserver::setTimeType(E::TimeType timeType)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		m_timeType = timeType;
		return;
	}

	bool TestObserver::setInitiator(int initialExpectationId)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		if (initialExpectationId < 0 || initialExpectationId >= std::ssize(m_expectations))
		{
			return false;
		}

		m_initialExpectationId = initialExpectationId;

		return true;
	}

	int TestObserver::addEqualExpectation(const QString& appSignalId, double expectedValue, double tolerance)
	{
		if (auto dataServers = m_signalDataServer.dataServiceIds(appSignalId);
			dataServers.isEmpty() == true)
		{
			return InvalidExpectationId;
		}

		m_expectations.push_back(std::make_unique<ToExpectationEqual>(appSignalId, expectedValue, tolerance));
		return static_cast<int>(m_expectations.size() - 1);
	}

	int TestObserver::addGreaterExpectation(const QString& appSignalId, double threshold)
	{
		if (auto dataServers = m_signalDataServer.dataServiceIds(appSignalId);
			dataServers.isEmpty() == true)
		{
			return InvalidExpectationId;
		}

		m_expectations.push_back(std::make_unique<ToExpectationGreater>(appSignalId, threshold));
		return static_cast<int>(m_expectations.size() - 1);
	}

	int TestObserver::addLessExpectation(const QString& appSignalId, double threshold)
	{
		if (auto dataServers = m_signalDataServer.dataServiceIds(appSignalId);
			dataServers.isEmpty() == true)
		{
			return InvalidExpectationId;
		}

		m_expectations.push_back(std::make_unique<ToExpectationLess>(appSignalId, threshold));
		return static_cast<int>(m_expectations.size() - 1);
	}

	int TestObserver::elapsedMs(const QString& appSignalId) const
	{
		for (const auto& e : m_expectations)
		{
			if (e->appSignalId == appSignalId)
			{
				return e->elapsedMs(m_waitPrivateData.metConditionsTime);
			}
		}

		return -1;
	}

	int TestObserver::expectationResult(int expectationId) const
	{
		if (expectationId < 0 || expectationId >= std::ssize(m_expectations))
		{
			return -1;
		}

		auto it = m_expectations.begin();
		std::advance(it, expectationId);

		int elapsedMs = (*it)->elapsedMs(m_waitPrivateData.metConditionsTime);
		return elapsedMs;
	}

	std::vector<int> TestObserver::expectations() const
	{
		std::vector<int> result(m_expectations.size());
		std::iota(result.begin(), result.end(), 0);

		return result;
	}

	QString TestObserver::expectationStr(int expectationId) const
	{
		QString result;

		if (expectationId >= 0 && expectationId < m_expectations.size())
		{
			auto it = m_expectations.begin();
			std::advance(it, expectationId);

			result = (*it)->toString();
		}
		else
		{
			result = QString("Wrong expectationId %1").arg(expectationId);
		}

		return result;
	}

	void TestObserver::dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem /*minState*/, TrendLib::TrendStateItem /*maxState*/)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		// First process data for initial condition, than iterate for all other.
		//
		if (m_waitPrivateData.initialConditionPtr != nullptr)
		{
			const std::list<TrendLib::RealtimeDataChunk>& signalData = data->signalData;

			auto it = std::find_if(signalData.begin(), signalData.end(), [hash = m_waitPrivateData.initialConditionPtr->appSignalHash](const TrendLib::RealtimeDataChunk& chunk)
								   {
									   return chunk.appSignalHash == hash;
								   });

			if (it != signalData.end())
			{
				const auto& chunk = *it;

				for (const TrendLib::TrendStateItem& state : chunk.states)
				{
					m_waitPrivateData.initialConditionPtr->fill(sourceEquipmentId, state);

					if (m_waitPrivateData.initialConditionPtr->met() == true)
					{
						// Bingo! We have met initial condition. Now we have to trim all data to this time point.
						//
						auto timePoint = state.getTime(m_timeType).timeStamp;

						m_waitPrivateData.initialConditionPtr = nullptr; // This indicates that initial condition is was met, so we can leave the loop.
						m_waitPrivateData.metConditionsTime = timePoint;

						// Trim states on all other conditions till timePoint.
						//
						for (auto& expectation : m_expectations)
						{
							if (expectation->operative.isInitial == true)
							{
								continue;
							}

							// Mark all as unsatisfied.
							// As they could be marked as satisfied right after start(), but before wait();
							//
							expectation->operative.satisfied = false;
							expectation->operative.metConditions = false;

							auto& states = expectation->operative.states; // Just short ref name.

							for (auto statesIt = states.begin(); statesIt != states.end();)
							{
								if (statesIt->getTime(m_timeType) >= timePoint)
								{
									break;
								}

								if (auto itNext = std::next(statesIt);
									itNext != states.end() && itNext->getTime(m_timeType) <= timePoint)
								{
									statesIt = states.erase(statesIt);
								}
								else
								{
									++statesIt;
								}
							}

							// Check for meeting expectation conditions after trim.
							//
							if (states.empty() == false && states.back().getTime(m_timeType) >= timePoint)
							{
								expectation->met();
							}
						}

						break;
					}
				}
			}
		}

		// Now process all other conditions, if initialConditionPtr is nullptr than initial condition is not present or it was satisfied.
		//
		for (const std::list<TrendLib::RealtimeDataChunk>& signalData = data->signalData;
			 const auto& chunk : signalData)
		{
			for (auto& e : m_expectations)
			{
				if (e->appSignalHash != chunk.appSignalHash)
				{
					continue;
				}

				// Process only unsatisfied expectations.
				//
				if (e->operative.satisfied == true || e->operative.isInitial == true)
				{
					continue;
				}

				for (const TrendLib::TrendStateItem& state : chunk.states)
				{
					if (m_waitPrivateData.metConditionsTime != 0 && state.getTime(m_timeType) < m_waitPrivateData.metConditionsTime)
					{
						continue;
					}

					// Add one point.
					//
					e->fill(sourceEquipmentId, state);

					// Check met() only if initial condition was already satisfied.
					//
					if (m_waitPrivateData.initialConditionPtr == nullptr && e->met() == true)
					{
						break; // Do not add any other points, as this particular point is a timestamp when the condition was satisfied.
					}
				}
			}
		}

		emit dataReceived();   // Wakeup TestObserver::waitPrivate(...).
		return;
	}

	void TestObserver::requestError(QString text)
	{
		if (m_logFile != nullptr)
		{
			m_logFile->writeError(tr("ScriptTestObserver: RtDataProvider request error: %1").arg(text));
		}

		return;
	}

	void TestObserver::connectionLost(QString sourceEquipmentId)
	{
		if (m_logFile != nullptr)
		{
			m_logFile->writeError(tr("ScriptTestObserver: RtDataProvider lost connection to AppDataService %1").arg(sourceEquipmentId));
		}

		return;
	}

	bool TestObserver::waitPrivate(int initialExpectationId, int timeoutMs)
	{
		Q_ASSERT(QThread::currentThread() == this->thread());

		if (m_rtDataProvide == nullptr)
		{
			return false;
		}

		if (initialExpectationId != InvalidExpectationId)
		{
			Q_ASSERT(initialExpectationId >= 0 || initialExpectationId < std::ssize(m_expectations));

			auto it = m_expectations.begin();
			std::advance(it, initialExpectationId);

			m_waitPrivateData.initialConditionPtr = *it;
			m_waitPrivateData.initialConditionPtr->operative.isInitial = true;
		}

		QDeadlineTimer deadline{timeoutMs};
		bool testResult = false;

		// Event loop ir required to receive signals for m_rtDataProvide.
		//
		QEventLoop eventLoop;

		// After each received data portion exit from the event loop.
		//
		QObject::connect(this, &TestObserver::dataReceived, &eventLoop, &QEventLoop::quit); // Leave the event loop when the data received.

		while (deadline.hasExpired() == false)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			// Check that all expectations has met condition.
			//
			{
				bool met = std::all_of(m_expectations.begin(), m_expectations.end(), [](const auto& e)
									   {
										   return e->operative.satisfied == true;
									   });
				if (met == true)
				{
					// Leave the wait loop. All done.
					//
					testResult = true;
					break;
				}
			}

			// Run message loop, which will populate data by calling slot TestObserver::dataReady.
			// TestObserver::dataReady erases expectations from m_expectationsLeft when they meet condition.
			// The loop will run until the timer timesout or new data is received.
			//
			QTimer::singleShot(deadline.remainingTime() > 200 ? 200 : deadline.remainingTime(), &eventLoop, &QEventLoop::quit);

			// Exit if timeout occurred or another portion of data received from m_rtDataProvide.
			//
			eventLoop.exec();
		}

		stop();

		return testResult;
	}
} // namespace TestSuite