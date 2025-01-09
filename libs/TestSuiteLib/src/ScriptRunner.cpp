#include <TestSuiteLib/ScriptRunner.h>

#include "ScriptRunnerImpl.h"

namespace TestSuite
{
	ScriptRunner::ScriptRunner(QString softwareEquipmentId,
							   const TestScript& script,
							   std::optional<TestScript> globalScript,
							   TestController& testController,
							   ILogFile& scriptTestLog,
							   ControlStatus& status,
							   QMutex& statusMutex) :
		m_impl{std::make_unique<ScriptRunnerImpl>(softwareEquipmentId,
												  script,
												  globalScript,
												  testController,
												  scriptTestLog,
												  status,
												  statusMutex)}
	{
		connect(m_impl.get(), &ScriptRunnerImpl::testStarted, this, &ScriptRunner::testStarted);
		connect(m_impl.get(), &ScriptRunnerImpl::testFinished, this, &ScriptRunner::testFinished);
		return;
	}

	ScriptRunner::~ScriptRunner() = default;

	bool ScriptRunner::queryPermission(bool& allowGlobal, bool& allowLocal)
	{
		return m_impl->queryPermission(allowGlobal, allowLocal);
	}

	bool ScriptRunner::runTests(const TestScriptSelection& filter)
	{
		return m_impl->runTests(filter);
	}

	const ScriptInfo& ScriptRunner::scriptInfo() const
	{
		return m_impl->scriptInfo();
	}

	const TestController& ScriptRunner::testController() const
	{
		return m_impl->testController();
	}

	QString ScriptRunner::plant() const
	{
		return m_impl->plant();
	}

	void ScriptRunner::setPlant(const QString& value)
	{
		m_impl->setPlant(value);
	}

	QString ScriptRunner::unit() const
	{
		return m_impl->unit();
	}

	void ScriptRunner::setUnit(const QString& value)
	{
		m_impl->setUnit(value);
	}

	QString ScriptRunner::system() const
	{
		return m_impl->system();
	}

	void ScriptRunner::setSystem(const QString& value)
	{
		m_impl->setSystem(value);
	}
} // namespace TestSuite
