#pragma once

#include "OutputController.h"
#include "InputController.h"
#include "TestLogController.h"
#include "TestScriptsStorage.h"
#include "../UtilsLib/ILogFile.h"

struct TestWorkerContext
{
	TestWorkerContext(OutputController* outputController, InputController* inputController, TestLogController* testLogController);

	// Controllers
	//
	OutputController* m_outputController = nullptr;
	InputController* m_inputController = nullptr;
	TestLogController* m_testLogController = nullptr;

	// Scripts to execute
	//
	std::vector<TestScript> scripts;
};

// TestWorker is a class that executes a test script by QJsEngine. It has access to full set  of controllers
//
class TestWorker : public QObject
{
	Q_OBJECT
public:
	TestWorker(const TestWorkerContext& context);

public slots:
	void run();

public:
	void stop();

signals:
	void finished();

private:
	TestWorkerContext m_context;

	std::unique_ptr<QJSEngine> m_jsEngine;

	std::atomic_bool m_jsStop = false;
};

// TestWorkerThread is a class that executes TestWorker in a separate thread
//
class TestWorkerThread : public QObject
{
	Q_OBJECT
public:
	TestWorkerThread(const TestWorkerContext& context, QObject* parent);

	void run();
	void stop();

	bool isRunning() const;

private slots:
	void workerFinished();

signals:
	void finished(int errorCode);

private:
	TestWorker m_testWorker;

	QThread m_thread;
};
