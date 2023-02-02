#include "TestWorker.h"

TestWorkerContext::TestWorkerContext(OutputController* outputController, InputController* inputController, TestLogController* testLogController):
	m_outputController(outputController),
	m_inputController(inputController),
	m_testLogController(testLogController)
{

}

//
// ----------------------------------- TestWorker -----------------------------
//
TestWorker::TestWorker(const TestWorkerContext& context):
	m_context(context)
{
	m_jsEngine = std::make_unique<QJSEngine>();

	m_jsEngine->installExtensions(QJSEngine::ConsoleExtension);

	//QJSValue jsBuilder = jsEngine->newQObject(this);
	//QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
	//QJSValue jsFirmware = jsEngine->newQObject(m_buildResultWriter->firmwareWriter());
	//QQmlEngine::setObjectOwnership(m_buildResultWriter->firmwareWriter(), QQmlEngine::CppOwnership);

	/*
	QJSValue jsEval = jsEngine->evaluate(contents);
	if (jsEval.isError() == true)
	{
		LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Module configuration script '%1' evaluation failed at line %2: %3").arg(lmDescription->configurationStringFile()).arg(jsEval.property("lineNumber").toInt()).arg(jsEval.toString()));
		return false;
	}

	if (!jsEngine->globalObject().hasProperty("main"))
	{
		LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Script has no \"main\" function"));
		return false;
	}

	if (!jsEngine->globalObject().property("main").isCallable())
	{
		LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("\"main\" property of script is not callable"));
		return false;
	}

	QJSValueList args;

	args << jsBuilder;
	args << jsRoot;

	QJSValue jsResult = jsEngine->globalObject().property("main").call(args);

	if (jsResult.isError() == true)
	{
		QString errorMessage = tr("Uncaught exception while generating module configuration '%1': %2, lineNumber: %3, Stack: %4, ")
							   .arg(lmDescription->configurationStringFile())
							   .arg(jsResult.toString())
							   .arg(jsResult.property("lineNumber").toInt())
							   .arg(jsResult.property("stack").toString());

		LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, errorMessage);
		return false;
	}

	if (jsResult.toBool() == false)
	{
		return false;
	}*/
}

void TestWorker::run()
{
	m_context.m_testLogController->addMessage("TestWorker::run");

	for (const TestScript& ts :  m_context.scripts)
	{
		m_context.m_testLogController->addMessage("Executing test: " + ts.fileName);


		for (int i = 0; i < 10; i++)
		{
			m_context.m_testLogController->addMessage(tr("Executing test: %1").arg(i));
			QThread::msleep(100);

			if (m_jsStop == true)
			{
				break;
			}
		}

		if (m_jsStop == true)
		{
			break;
		}
	}


	if (m_jsStop == true)
	{
		m_context.m_testLogController->addMessage("TestWorker::execution interrupted");
	}
	else
	{
		m_context.m_testLogController->addMessage("TestWorker::finished");
	}

	emit finished();
}

void TestWorker::stop()
{
	m_jsStop = true;
}

//
// ----------------------------------- TestWorkerThread -----------------------------
//

TestWorkerThread::TestWorkerThread(const TestWorkerContext& context, QObject *parent):
	m_testWorker(context)
{
	Q_UNUSED(parent);

	m_testWorker.moveToThread(&m_thread);

	QObject::connect(&m_thread, &QThread::started, &m_testWorker, &TestWorker::run);
	QObject::connect(&m_testWorker, &TestWorker::finished, this, &TestWorkerThread::workerFinished);

	return;
}

void TestWorkerThread::run()
{
	if (m_thread.isRunning() == true)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread.start();

	return;
}

void TestWorkerThread::stop()
{
	m_testWorker.stop();

	return;
}

bool TestWorkerThread::isRunning() const
{
	return m_thread.isRunning();
}

void TestWorkerThread::workerFinished()
{
	m_thread.quit();
	m_thread.wait();

	emit finished(0);

	return;
}
