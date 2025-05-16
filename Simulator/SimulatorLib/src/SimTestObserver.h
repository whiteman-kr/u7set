#pragma once

#include "../AppSignalLib/IAppSignalManager.h"
#include <ClientLib/ITestObserver.h>

namespace Sim
{
	class SimulatorPrivate;

	struct ToExpectation
	{
		ToExpectation() = delete;
		ToExpectation(const QString& appSignalId) :
			appSignalId{appSignalId}
		{
		}

		virtual ~ToExpectation() = default;

		virtual void fill(const IAppSignalManager& signalManager)
		{
			bool hasState = false;
			AppSignalState state = signalManager.signalState(::calcHash(appSignalId), &hasState);

			if (hasState == false || state.isStateAvailable() == false)
			{
				return;
			}

			// Add only really changed states.
			//
			if (states.empty() == true)
			{
				states.push_back(state);
				return;
			}

			const auto& lastState = states.back();
			if (lastState.value() != state.value() || lastState.m_flags.all != state.m_flags.all)
			{
				states.push_back(state);
			}

			return;
		}

		bool met()
		{
			metConditions = std::max(metPrivate(), metConditions);
			return metConditions;
		}

		virtual bool metPrivate() const = 0;

		/// @brief Elapsed time in milliseconds until expectation was fulfilled, or -1 if the expectation conditions were not met.
		int elapsedMs() const
		{
			return metConditions ?
					   states.back().time(E::TimeType::Plant).timeStamp - states.front().time(E::TimeType::Plant).timeStamp :
					   -1;
		}

		virtual QString toString() const = 0;

		// Data
		//
		const QString appSignalId;

		std::deque<AppSignalState> states;
		bool metConditions = false;
	};

	struct ToExpectationEqual : ToExpectation
	{
		ToExpectationEqual(const QString& appSignalId, double expectedValue, double tolerance) :
			ToExpectation{appSignalId},
			expectedValue{expectedValue},
			tolerance{tolerance}
		{
		}

		double expectedValue{};
		double tolerance{};

		bool metPrivate() const override
		{
			if (states.empty() == true)
			{
				Q_ASSERT(states.empty() == false);
				return false;
			}

			double value = states.back().value();

			if (std::isnan(expectedValue) == true && std::isnan(value) == true)
			{
				return true;
			}

			if (std::isinf(expectedValue) == true && std::isinf(value) == true && std::signbit(expectedValue) == std::signbit(value))
			{
				return true;
			}

			return std::abs(expectedValue - value) <= tolerance;
		}

		virtual QString toString() const override
		{
			return QString("%1 == %2 (tolerance %3)").arg(appSignalId).arg(expectedValue).arg(tolerance);
		}
	};

	struct ToExpectationGreater : ToExpectation
	{
		ToExpectationGreater(const QString& appSignalId, double threshold) :
			ToExpectation{appSignalId},
			threshold{threshold}
		{
		}

		double threshold{};

		bool metPrivate() const override
		{
			Q_ASSERT(states.empty() == false);
			return states.back().value() > threshold;
		}

		virtual QString toString() const override
		{
			return QString("%1 > %2").arg(appSignalId).arg(threshold);
		}
	};

	struct ToExpectationLess : ToExpectation
	{
		ToExpectationLess(const QString& appSignalId, double threshold) :
			ToExpectation{appSignalId},
			threshold{threshold}
		{
		}

		double threshold{};

		bool metPrivate() const override
		{
			return states.back().value() < threshold;
		}

		virtual QString toString() const override
		{
			return QString("%1 < %2").arg(appSignalId).arg(threshold);
		}
	};


	/// @brief Implementation of the TestObserver for Simulator.
	///
	class TestObserver : public ITestObserver
	{
	public:
		TestObserver() = delete;
		TestObserver(SimulatorPrivate& simulator);

		// ITestObserver implementation.
		//
	public:
		virtual bool start() override;
		virtual void stop() override;
		virtual void clear() override;

		virtual bool wait(int timeoutMs) override;

		virtual void setTimeType(E::TimeType timeType) override;
		virtual bool setInitiator(int initialExpectationId) override;

		virtual int addEqualExpectation(const QString& appSignalId, double expectedValue, double tolerance) override;
		virtual int addGreaterExpectation(const QString& appSignalId, double threshold) override;
		virtual int addLessExpectation(const QString& appSignalId, double threshold) override;

		virtual int elapsedMs(const QString& appSignalId) const override;
		virtual int expectationResult(int expectationId) const override;

		virtual std::vector<int> expectations() const override;
		virtual QString expectationStr(int expectationId) const override;

		// End of ITestObserver

	private:
		bool waitPrivate(std::list<ToExpectation*> expectations, int timeoutMs);

	private:
		SimulatorPrivate& m_simulator;

		std::list<std::unique_ptr<ToExpectation>> m_expectations; // expectationId is an index in m_expectation.
		int m_initialExpectationId = InvalidExpectationId;
	};
} // namespace Sim