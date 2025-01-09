#pragma once

#include <chrono>

namespace TestSuite
{
	enum class ControlState
	{
		// TestControlState states
		//
		Stop,
		RequestingConfiguration,
		InitInputController,
		InitOutputController,
		RunningTests,
		CreatingReports
	};

	class ControlStatus
	{
	public:
		ControlState m_state = ControlState::Stop;

		QString m_scriptFile;
		qsizetype m_scriptIndex{0};
		qsizetype m_scriptCount{0};

		QString m_testFunction;
		qsizetype m_testIndex{0};
		qsizetype m_testCount{0};

	public:
		void setStartTime()
		{
			using namespace std::chrono;
			m_startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		}

		std::chrono::milliseconds duration()
		{
			using namespace std::chrono;
			auto m_currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			return m_currentTime - m_startTime;
		}

		void reset() { *this = ControlStatus{}; }

	private:
		std::chrono::milliseconds m_startTime{0}; // Test start time, default value is 0
												  // Suggestion: Use steady_clock::time_point.
	};
} // namespace TestSuite
