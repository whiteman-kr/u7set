#pragma once

#include <chrono>

namespace TestSuite
{
	enum class ControlState
	{
		//TestControlState states
		//
		Stop,
		RequestingConfiguration,
		InitInputController,
		InitOutputController,
		RunningTests,
		CreatingReports
	};

	struct ControlStatus
	{
		ControlStatus() = default;

		ControlState m_state = ControlState::Stop;

		QString m_scriptFile;
		qsizetype m_scriptIndex{0};
		qsizetype m_scriptCount{0};

		QString m_testFunction;
		qsizetype m_testIndex{0};
		qsizetype m_testCount{0};

	private:
		std::chrono::milliseconds m_startTime{0};	// Test start time, default value is 0

	public:
		void setStartTime()
		{
			m_startTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		}

		std::chrono::milliseconds duration()
		{
			std::chrono::milliseconds m_currentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			return m_currentTime - m_startTime;
		}

		void reset()
		{
			m_state = ControlState::Stop;

			m_scriptFile.clear();
			m_scriptIndex = 0;
			m_scriptCount = 0;

			m_testFunction.clear();
			m_testIndex = 0;
			m_testCount = 0;

		}
	};
}
