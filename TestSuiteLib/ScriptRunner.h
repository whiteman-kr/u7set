#pragma once

#include "TestController.h"
#include "ScriptTestLog.h"
#include "TestScriptsStorage.h"
#include "../UtilsLib/ILogFile.h"
#include "ControlState.h"

namespace TestSuite
{


	struct ScriptInfo
	{
		QString scriptCaption;

		QStringList functionsList;
		std::map<QString, QString> functionsCaptions;	// Key is function name, value is function caption

		qsizetype count() const
		{
			return functionsList.size();
		}

		QString functionCaption(const QString& function, bool* found = nullptr)
		{
			auto it = functionsCaptions.find(function);
			if (it == functionsCaptions.end())
			{
				if (found != nullptr)
				{
					*found = false;
				}
				return function;
			}
			if (found != nullptr)
			{
				*found = true;
			}
			return it->second;
		}
	};


	// ScriptRunner is a class that executes a test script by QJSEngine.
	//
	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(ConfigSettings& configuration, TestController& testController, ILogFile& scriptTestLog, ControlStatus& status, QMutex& statusMutex);
		virtual ~ScriptRunner();

	public:
		bool getScriptTestList(const TestScript& script, ScriptInfo& scriptInfo, QString& errorMsg);
		bool runScript(const TestScript& script, const TestScript* globalScript, const TestScriptSelection& filter);

	signals:
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	private:
		bool evaluateScript(const TestScript& script, const TestScriptSelection& filter, ScriptInfo& scriptInfo, QString& errorMsg);
		bool runScriptFunction(const QString& functionName);

	private:
		ConfigSettings& m_configuration;
		TestController& m_testController;
		ScriptTestLog m_scriptTestLog;

		QJSEngine m_jsEngine;
		QJSValue m_jsTestController;
		QJSValue m_jsLog;

		ControlStatus& m_status;
		QMutex& m_statusMutex;
	};
}
