#pragma once

#include "TestController.h"
#include "ScriptTestLog.h"
#include "TestScriptsStorage.h"
#include "../UtilsLib/ILogFile.h"
#include "ControlState.h"

#include <QJSEngine>

namespace TestSuite
{


	struct ScriptInfo
	{
		QString fileName;
		QString scriptCaption;
		QStringList scriptTags;

		QStringList testsList;
		std::map<QString, QString> testsCaptions;	// Key is function name, value is function caption

		QString globalAllowFunction;	// Name of global allow function (allowGlobal())
		QString allowFunction;			// Name of local allow function (allow<SCRIPT_FILE_NAME>())

		ScriptInfo(const QString& fileName)
		{
			this->fileName = fileName;
		}

		bool empty() const
		{
			return testsList.empty();	// No functions are in this script. Possibly, it is not evaluated
		}

		qsizetype testsCount() const
		{
			return testsList.size();
		}

		QString testCaption(const QString& function, bool* found = nullptr) const
		{
			auto it = testsCaptions.find(function);
			if (it == testsCaptions.end())
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

		bool checkScriptTags(const QString& tagsProperty) const
		{
			if (scriptTags.isEmpty() == false)
			{
				bool tagFound = false;
				for (const QString& tag : scriptTags)
				{
					if (tagsProperty.contains(tag) == true)
					{
						// Tag was found
						tagFound = true;
						break;
					}
				}
				if (tagFound == false)
				{
					return false;
				}
			}
			return true;
		}
	};


	// ScriptRunner is a class that executes a test script by QJSEngine.
	//
	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(const TestScript& script, const TestScript* globalScript, ConfigSettings& configuration, TestController& testController, ILogFile& scriptTestLog, ControlStatus& status, QMutex& statusMutex);
		virtual ~ScriptRunner();

	public:
		bool queryPermission(bool& allowGlobal, bool& allowLocal);
		bool runTests(const TestScriptSelection& filter);

		const ScriptInfo& scriptInfo() const;
		const TestController& testController() const;

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	private:
		bool evaluateScript(const TestScript& script, ScriptInfo& scriptInfo, QString& errorMsg);
		bool runScriptFunction(const QString& functionName);

	private:
		ConfigSettings& m_configuration;
		TestController& m_testController;
		ScriptTestLog m_scriptTestLog;

		ScriptInfo m_scriptInfo;

		QJSEngine m_jsEngine;
		QJSValue m_jsTestController;
		QJSValue m_jsLog;

		ControlStatus& m_status;
		QMutex& m_statusMutex;
	};
}
