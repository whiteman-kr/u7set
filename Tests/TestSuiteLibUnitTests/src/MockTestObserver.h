#pragma once

#include <ClientLib/ITestObserver.h>


class MockTestObserver : public ITestObserver
{
public:
	MOCK_METHOD(bool, start, (), (override));
	MOCK_METHOD(void, stop, (), (override));
	MOCK_METHOD(void, clear, (), (override));
	MOCK_METHOD(bool, wait, (int timeoutMs), (override));
	MOCK_METHOD(void, setTimeType, (E::TimeType timeType), (override));
	MOCK_METHOD(bool, setInitiator, (int initialExpectationId), (override));
	MOCK_METHOD(int, addEqualExpectation, (const QString& appSignalId, double expectedValue, double tolerance), (override));
	MOCK_METHOD(int, addGreaterExpectation, (const QString& appSignalId, double threshold), (override));
	MOCK_METHOD(int, addLessExpectation, (const QString& appSignalId, double threshold), (override));
	MOCK_METHOD(int, elapsedMs, (const QString& appSignalId), (const, override));
	MOCK_METHOD(int, expectationResult, (int expectationId), (const, override));
	MOCK_METHOD(std::vector<int>, expectations, (), (const, override));
	MOCK_METHOD(QString, expectationStr, (int expectationId), (const, override));
};
