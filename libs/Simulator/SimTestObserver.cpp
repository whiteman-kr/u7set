#include "SimTestObserver.h"
#include "SimulatorPrivate.h"

namespace Sim
{
	TestObserver::TestObserver(SimulatorPrivate& simulator) :
		m_simulator{simulator}
	{
	}

	bool TestObserver::start()
	{
		return true;
	}

	void TestObserver::stop()
	{
	}

	void TestObserver::clear()
	{
		m_expectations.clear();
		m_initialExpectationId = InvalidExpectationId;
	}

	bool TestObserver::wait(int timeoutMs)
	{
		std::list<ToExpectation*> expectations;

		for (auto& e : m_expectations)
		{
			expectations.push_back(e.get());
		}

		if (m_initialExpectationId != InvalidExpectationId)
		{
			// Initiator was set, wait for it.
			//
			std::list<ToExpectation*> initialExpectation;

			auto it = expectations.begin();
			std::advance(it, m_initialExpectationId);

			initialExpectation.splice(initialExpectation.end(), expectations, it); // Move from list expectations, item initialExpectationId.

			bool ok = waitPrivate(std::move(initialExpectation), timeoutMs);
			if (ok == false)
			{
				return false;
			}

			timeoutMs -= expectationResult(m_initialExpectationId);
		}

		return waitPrivate(std::move(expectations), timeoutMs);
	}

	void TestObserver::setTimeType(E::TimeType /*timeType*/)
	{
	}

	bool TestObserver::setInitiator(int initialExpectationId)
	{
		if (initialExpectationId < 0 || initialExpectationId >= std::ssize(m_expectations))
		{
			return false;
		}

		m_initialExpectationId = initialExpectationId;

		return true;
	}

	int TestObserver::addEqualExpectation(const QString& appSignalId, double expectedValue, double tolerance)
	{
		bool signalExists = m_simulator.appSignalManager().signalExists(appSignalId);
		if (signalExists == false)
		{
			return InvalidExpectationId;
		}

		m_expectations.push_back(std::make_unique<ToExpectationEqual>(appSignalId, expectedValue, tolerance));
		return static_cast<int>(m_expectations.size() - 1);
	}

	int TestObserver::addGreaterExpectation(const QString& appSignalId, double threshold)
	{
		bool signalExists = m_simulator.appSignalManager().signalExists(appSignalId);
		if (signalExists == false)
		{
			return InvalidExpectationId;
		}

		m_expectations.push_back(std::make_unique<ToExpectationGreater>(appSignalId, threshold));
		return static_cast<int>(m_expectations.size() - 1);
	}

	int TestObserver::addLessExpectation(const QString& appSignalId, double threshold)
	{
		bool signalExists = m_simulator.appSignalManager().signalExists(appSignalId);
		if (signalExists == false)
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
				return e->elapsedMs();
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

		int elapsedMs = (*it)->elapsedMs();
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

	bool TestObserver::waitPrivate(std::list<ToExpectation*> expectations, int timeoutMs)
	{
		if (m_simulator.isRunning() == true)
		{
			m_simulator.control().pause();
		}

		auto lms = m_simulator.logicModules();
		auto minWorkcycleLm = *std::ranges::min_element(lms, {}, [](const auto& lm)
														{
															return lm->cycleDuration();
														});
		const auto minWorkcycle = minWorkcycleLm->cycleDuration();


		while (expectations.empty() == false && timeoutMs >= 0) // timeoutMs >= 0 : check value at least once.
		{
			// Populate data
			//
			for (ToExpectation* e : expectations)
			{
				e->fill(m_simulator.appSignalManager());
			}

			// Check expectations
			//
			expectations.remove_if([](ToExpectation* e)
								   {
									   return e->met();
								   });

			if (expectations.empty() == true || timeoutMs == 0) // timeoutMs == 0 : no need to call startSimulation().
			{
				break;
			}

			timeoutMs -= std::chrono::duration_cast<std::chrono::milliseconds>(minWorkcycle).count();

			// Start and stop simulator by one workcycle.
			//
			bool ok = m_simulator.control().startSimulation(minWorkcycle);
			if (ok == false)
			{
				return false;
			}

			// Wait for finishing simulation
			//
			while (m_simulator.control().isRunning() == true)
			{
				QThread::yieldCurrentThread();
			}
		}

		return expectations.empty();
	}
} // namespace Sim