#pragma once
#include "ITestObserver.h"
#include "../UtilsLib/ILogFile.h"

/// @class ScriptTestObserver
/// @ingroup testsuite simulator
/// @brief The ScriptTestObserver class provides functionality to observe and analyze test results through expectations.
///
/// This class enables precise observation of signal behavior against defined expectations.
/// It supports various expectations, such as equality, greater-than, and less-than comparisons.
/// Users can establish initial conditions, wait for expectations, or handle timeouts.
/// If an initial condition is set, the time measurement starts from the fulfillment of the initial expectation.
/// Otherwise, the time measurement starts from the beginning of the observation.
/// The Observer object can be reused after calling the clear() function to reset expectations and free resources.
///
/// Example of usage :
/// @code
/// // Create observer.
/// let observer = ctrl.createObserver();
///
/// // Add expectation which will be initial condition.
/// let initiatorId = observer.addEqualExpectation("#INPUT", 1);
/// observer.setInitiator(initiatorId);
///
/// // Add expectations for measure time.
/// observer.addEqualExpectation("#OUTPUT1", 1); // Wait this signal to become 1.
/// observer.addEqualExpectation("#OUTPUT2", 0); // Wait this signal to become 0.
///
/// // Observer connects to AppDataService for signal state retrieval, start of measurements.
/// observer.start();
///
/// ctrl.overrideSignalValue("#INPUT", 1); // Set initial signal.
///
/// let waitResult = observer.wait(5000);  // Wait for satisfying all three expectations.
/// assert(waitResult);                    // returns true if all expectations were fulfilled.
///
/// assert(observer.elapsedMs("#OUTPUT1") === 50);  // Expected signal #OUTPUT1 to become 1 after 50 ms.
/// assert(observer.elapsedMs("#OUTPUT2") === 100); // Expected signal #OUTPUT2 to become 0 after 100 ms.
///
/// @endcode
class ScriptTestObserver : public QObject
{
	Q_OBJECT

public:
	ScriptTestObserver() = delete;
	ScriptTestObserver(std::unique_ptr<ITestObserver> observer, ILogFile* logFile, QObject* parent);

public slots:
	/// @brief Start all the threads and waits until all connections are established.
	bool start();

	/// @brief Stops data accumulation and analysis. Users can retrieve results and analyze data.
	void stop();

	/// @brief Stops all threads and releases all resources.
	void clear();

	/// @brief Blocks and waits for an initial condition (if was set), then waits until all other added expectations are met or a timeout occurs.
	/// @param timeout Timeout duration in milliseconds.
	/// @return `true` if all expectations were satisfied, or `false` if not.
	bool wait(int timeoutMs);

	/// @brief Use plant time  (time received from LogicModule) for time measurements.
	///        This option is recommended for single-module tests.
	///        This option is set by default.
	void usePlantTime();

	/// @brief Use system time (server time, assigned when LogicModule data is received) for time measurements.
	///        This option is recommended when tested signals are from different LogicModules and/or plant time is not stable.
	///        The default option is to use plant time.
	void useLocalTime();

	/// @brief Set an initial condition based on a previously added expectation. Throws an exception if initiator was not set.
	/// @param initialExpectationId The identifier of an added expectation using an add* function.
	void setInitiator(int initialExpectationId);

	/// @brief Adds an expectation for signal to be nearly equal to the specified value within a given tolerance range.
	/// @param appSignalId Application signal identifier.
	/// @param expectedValue Expected value.
	/// @param tolerance Range within which the actual signal value can differ from the expected value.
	/// @return A positive value represents the added Expectation Identifier; -1 indicates an error.
	int addEqualExpectation(QString appSignalId, double expectedValue, double tolerance = 0);

	/// @brief Adds an expectation that a signal will be greater than the specified value.
	/// @param appSignalId Application signal identifier.
	/// @param threshold Threshold value.
	/// @return A positive value represents the added Expectation Identifier; -1 indicates an error.
	int addGreaterExpectation(QString appSignalId, double threshold);

	/// @brief Adds an expectation that a signal will be less than the specified value.
	/// @param appSignalId Application signal identifier.
	/// @param threshold Threshold value.
	/// @return A positive value represents the added Expectation Identifier; -1 indicates an error.
	int addLessExpectation(QString appSignalId, double threshold);

	/// @brief Retrieves the elapsed time in milliseconds until an expectation was fulfilled for the specified signal. Note, if there are more then one expectation for a signal, then the first added expectation result is returned.
	/// @param appSignalId Application signal identifier for obtaining expectation result.
	/// @return Elapsed time in milliseconds until expectation was fulfilled, or -1 if the expectation conditions were not met.
	int elapsedMs(QString appSignalId) const;

	/// @brief Retrieves the elapsed time in milliseconds until an expectation was fulfilled for the specified expectationId.
	/// @param expectationId Expectation identifier returned by addEqualExpectation, addGreaterExpectation, or addLessExpectation.
	/// @return Elapsed time in milliseconds until expectation was fulfilled, or -1 if the expectation conditions were not met.
	int expectationResult(int expectationId) const;

private:
	void reportError(const QString& message, bool throwException);

private:
	std::unique_ptr<ITestObserver> m_observer;
	ILogFile* m_logFile = nullptr;
};