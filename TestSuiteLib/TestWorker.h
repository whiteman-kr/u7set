#pragma once

#include "OutputController.h"
#include "InputController.h"
#include "TestLogController.h"

// TestWorker is a class that executes a test script by QJsEngine. It has access to full set  of controllers

class TestWorker : public QObject
{
	Q_OBJECT
public:
	TestWorker(OutputController* outputController,
			   InputController* inputController,
			   TestLogController* testLogController,
			   const QString& testScript,
			   QObject* parent);


	void exec();


signals:
	void finished();

private:
	OutputController* m_outputController = nullptr;
	InputController* m_inputController = nullptr;
	TestLogController* m_testLogController = nullptr;

	QString m_testScript;

	std::unique_ptr<QJSEngine> m_jsEngine;
};

