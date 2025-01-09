#pragma once

#include <TestSuiteLib/ScriptInfo.h>
#include <TestSuiteLib/TestScriptSelection.h>
#include <TestSuiteLib/TestScript.h>

#include <QJSEngine>
#include <QMutex>


class ILogFile;
class ScriptTestSuiteApplication;


namespace TestSuite
{
	class TestController;
	class ControlStatus;
	class ScriptTestLog;

	enum class RunScriptError
	{
		Success,
		FunctionNotDefined,
		IsNotCallable,
		RuntimeError,
		ReturnedError // Function returned false
	};


	class ScriptRunnerImpl : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunnerImpl(QString softwareEquipmentId,
						 const TestScript& script,
						 std::optional<TestScript> globalScript,
						 TestController& testController,
						 ILogFile& scriptTestLog,
						 ControlStatus& status,
						 QMutex& statusMutex);
		virtual ~ScriptRunnerImpl();

	public:
		[[nodiscard]] bool queryPermission(bool& allowGlobal, bool& allowLocal);
		bool runTests(const TestScriptSelection& filter);

		[[nodiscard]] const ScriptInfo& scriptInfo() const;
		const TestController& testController() const;

		[[nodiscard]] QString plant() const;
		void setPlant(const QString& value);

		[[nodiscard]] QString unit() const;
		void setUnit(const QString& value);

		[[nodiscard]] QString system() const;
		void setSystem(const QString& value);


	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	private:
		bool evaluateScript(const TestScript& script, ScriptInfo& scriptInfo, QString& errorMsg);

		RunScriptError runScriptFunction(const QString& functionName);

	private:
		QString m_plant;
		QString m_unit;
		QString m_system;

		TestController& m_testController;
		std::unique_ptr<ScriptTestLog> m_scriptTestLog;
		std::unique_ptr<::ScriptTestSuiteApplication> m_app;

		ScriptInfo m_scriptInfo;

		QJSEngine m_jsEngine;
		QJSValue m_jsTestController;
		QJSValue m_jsLog;
		QJSValue m_jsApp;

		ControlStatus& m_status;
		QMutex& m_statusMutex;
	};
} // namespace TestSuite