#pragma once

#include "ControlStatus.h"
#include "ScriptInfo.h"
#include "TestController.h"
#include "TestScriptSelection.h"
#include "TestScriptsStorage.h"

#include "../UtilsLib/ILogFile.h"

#include <QMutex>

#include <memory>
#include <optional>


namespace TestSuite
{
	class ScriptTestLog;
	class ScriptRunnerImpl;


	// ScriptRunner is a class that executes a test script by QJSEngine.
	//
	class ScriptRunner : public QObject
	{
		Q_OBJECT

	public:
		ScriptRunner(QString softwareEquipmentId,
					 const TestScript& script,
					 std::optional<TestScript> globalScript,
					 TestController& testController,
					 ILogFile& scriptTestLog,
					 ControlStatus& status,
					 QMutex& statusMutex);
		virtual ~ScriptRunner();

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
		std::unique_ptr<ScriptRunnerImpl> m_impl;
	};
} // namespace TestSuite
