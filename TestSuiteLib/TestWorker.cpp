#include "TestWorker.h"

TestWorker::TestWorker(OutputController* outputController,
					   InputController* inputController,
					   TestLogController* testLogController,
					   const QString& testScript,
					   QObject *parent):
	QObject(parent),
	m_outputController(outputController),
	m_inputController(inputController),
	m_testLogController(testLogController),
	m_testScript(testScript)
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
