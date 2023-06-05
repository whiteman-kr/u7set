#pragma once

#include "TestController.h"
#include "ScriptTestLog.h"
#include "TestScriptsStorage.h"
#include "../UtilsLib/ILogFile.h"
#include "ControlState.h"

namespace TestSuite
{
	// ScriptRunner is a class that executes a test script by QJSEngine.
	//
	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(TestController& testController, ITestLog& scriptTestLog, ControlStatus& status, QMutex& statusMutex);
		virtual ~ScriptRunner();

	public:
		bool runScript(const TestScript& script, const QString& functionsFilter);

	private:
		bool runScriptFunction(const QString& functionName);

	private:
		TestController& m_testController;
		ScriptTestLog m_scriptTestLog;

		QJSEngine m_jsEngine;
		QJSValue m_jsTestController;
		QJSValue m_jsLog;

		ControlStatus& m_status;
		QMutex& m_statusMutex;
	};
}
