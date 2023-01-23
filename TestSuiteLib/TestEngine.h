#pragma once

#include "InputController.h"
#include "OutputController.h"
#include "TestLogController.h"
#include "TestWorker.h"
#include "TestResultLog.h"


class TestEngineThread
{

};

class TestEngine : public QObject
{
	Q_OBJECT
public:
	TestEngine();


	void start();
	void stop();

	bool isRunning() const;

signals:
	void finished(int result);

public:
	const TestResultLog& testResultLog() const;

private:
	InputController* m_inputController = nullptr;
	OutputController* m_outputController = nullptr;
	TestWorker* m_testWorker = nullptr;
	TestLogController* m_testLogController = nullptr;

	TestResultLog m_testResultLog;
};

