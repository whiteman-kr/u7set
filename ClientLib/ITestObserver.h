#pragma once
#include <vector>

/// @class ITestObserver
/// @brief An interface for observing and managing test expectations.
class ITestObserver
{
public:
	virtual ~ITestObserver() = default;

	virtual bool start() = 0;
	virtual void stop() = 0;
	virtual void clear() = 0;

	virtual bool wait(int timeoutMs) = 0;

	virtual void setTimeType(E::TimeType timeType) = 0;
	virtual bool setInitiator(int initialExpectationId) = 0;

	virtual int addEqualExpectation(const QString& appSignalId, double expectedValue, double tolerance) = 0;
	virtual int addGreaterExpectation(const QString& appSignalId, double threshold) = 0;
	virtual int addLessExpectation(const QString& appSignalId, double threshold) = 0;

	virtual int elapsedMs(const QString& appSignalId) const = 0;
	virtual int expectationResult(int expectationId) const = 0;

	virtual std::vector<int> expectations() const = 0;
	virtual QString expectationStr(int expectationId) const = 0;

	static const int InvalidExpectationId = -1;
};