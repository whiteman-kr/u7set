#pragma once

namespace TestSuite
{
	enum class ControlState
	{
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

		//std::chrono::microseconds m_startTime = 0us;	// When testing was started, it's computer time
		//std::chrono::microseconds m_currentTime = 0us;	// Current time in testing

		//std::chrono::microseconds m_duration{0};
		ControlState m_state = ControlState::Stop;

		QString m_scriptFile;
		qsizetype m_scriptIndex{0};
		qsizetype m_scriptCount{0};

		QString m_testFunction;
		qsizetype m_testIndex{0};
		qsizetype m_testCount{0};

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
