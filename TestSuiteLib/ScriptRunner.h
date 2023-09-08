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
		ScriptRunner(TestController& testController, ILogFile& scriptTestLog, ControlStatus& status, QMutex& statusMutex);
		virtual ~ScriptRunner();

	public:
		bool getScriptTestFunctions(const TestScript& script, QStringList& functionsList, QString& errorMsg);
		bool runScript(const TestScript& script, const TestScript* globalScript, const TestScriptSelection& filter);

	signals:
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	private:
		bool evaluateScript(const TestScript& script, const TestScriptSelection& filter, QStringList& functionsList, QString& errorMsg);
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
