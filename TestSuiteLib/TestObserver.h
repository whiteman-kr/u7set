#pragma once

#include <deque>

#include <ClientLib/ITestObserver.h>
#include <ClientLib/RtDataProvider.h>
#include "../OnlineLib/SoftwareSettings.h"

namespace TestSuite
{
	struct ToExpectation
	{
		ToExpectation() = delete;

		ToExpectation(const QString& appSignalId) :
			appSignalId{appSignalId},
			appSignalHash{::calcHash(appSignalId)}
		{
		}

		virtual ~ToExpectation() = default;

		virtual void fill(const QString& sourceEquipmentId, const TrendLib::TrendStateItem& state)
		{
			// No use for non valid points
			//
			if (state.isValid() == false)
			{
				return;
			}

			if (operative.dataSourceEquipmentId.isEmpty() == true)
			{
				// This is the first valid point, fixate source for using only it in future.
				//
				operative.dataSourceEquipmentId = sourceEquipmentId;
			}

			if (operative.dataSourceEquipmentId != sourceEquipmentId)
			{
				// This is an another source for realtime data, for tests we use only one source - the first valid arrived.
				//
				return;
			}

			operative.states.push_back(state);
			return;
		}

		bool met()
		{
			if (operative.metConditions == true)
			{
				return true;
			}

			operative.metConditions = metPrivate();

			if (operative.metConditions == true)
			{
				operative.satisfied = true;
			}

			return operative.metConditions;
		}

		virtual bool metPrivate() const = 0;

		/// @brief Elapsed time in milliseconds until expectation was fulfilled, or -1 if the expectation conditions were not met.
		int elapsedMs(qint64 metConditionsTime) const
		{
			if (metConditionsTime == 0)
			{
				// Possible there was not initial condition.
				//
				return (operative.metConditions && operative.states.empty() == false) ?
						   operative.states.back().plant - operative.states.front().plant :
						   -1;
			}
			else
			{
				return (operative.metConditions && operative.states.empty() == false) ?
						   operative.states.back().plant - metConditionsTime :
						   -1;
			}
		}

		virtual QString toString() const = 0;

		// Data
		//
		const QString appSignalId;
		const Hash appSignalHash{};

		struct OperativeData
		{
			QString dataSourceEquipmentId; // Several realtime data sources can be configured, but we take the only one, the first arrived.
			std::deque<TrendLib::TrendStateItem> states;
			bool metConditions = false;
			bool satisfied = false;
			bool isInitial = false;
		} operative;
	};

	struct ToExpectationEqual : ToExpectation
	{
		ToExpectationEqual(const QString& appSignalId, double expectedValue, double tolerance) :
			ToExpectation{appSignalId},
			expectedValue{expectedValue},
			tolerance{tolerance}
		{
		}

		const double expectedValue{};
		const double tolerance{};

		bool metPrivate() const override
		{
			if (operative.states.empty() == true)
			{
				return false;
			}

			double value = operative.states.back().value;

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
			if (operative.states.empty() == true)
			{
				return false;
			}

			return operative.states.back().value > threshold;
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
			if (operative.states.empty() == true)
			{
				return false;
			}

			return operative.states.back().value < threshold;
		}

		virtual QString toString() const override
		{
			return QString("%1 < %2").arg(appSignalId).arg(threshold);
		}
	};


	/// @brief Implementation of the TestObserver for TestSuite.
	///
	class TestObserver : public QObject,
						 public ITestObserver
	{
		Q_OBJECT

	public:
		TestObserver() = delete;
		TestObserver(ISignalDataServer& signalDataServer,
					 const SoftwareInfo& softwareInfo,
					 const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
					 ILogFile* logFile);

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

	private slots:
		void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

	private:
		bool waitPrivate(int initialExpectationId, int timeoutMs);

	signals:
		void dataReceived();

	private:
		ISignalDataServer& m_signalDataServer;
		const SoftwareInfo m_softwareInfo;
		std::vector<SoftwareEndpoint::AppDataService> m_appDataServices;
		ILogFile* m_logFile = nullptr;

		std::unique_ptr<ClientLib::RtDataProvider> m_rtDataProvide;

		E::TimeType m_timeType = E::TimeType::Plant;
		std::list<std::shared_ptr<ToExpectation>> m_expectations; // expectationId is an index in m_expectation.
		int m_initialExpectationId = ITestObserver::InvalidExpectationId;

		struct WaitPrivateData
		{
			std::shared_ptr<ToExpectation> initialConditionPtr;   // Initial condition expectation.
			qint64 metConditionsTime{};
		} m_waitPrivateData;

		static constexpr auto s_defaultSamplePeriod = E::RtTrendsSamplePeriod::sp_5ms;
		static constexpr auto s_connectionTimeout = std::chrono::milliseconds{3000};
	};
} // namespace TestSuite