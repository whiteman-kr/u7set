#include <Simulator/Simulator.h>
#include "SimulatorPrivate.h"


namespace Sim
{
	//
	// Simulator
	//
	Simulator::Simulator(ILogFile* log, bool allowDebugMessages, QObject* parent) :
		QObject{parent},
		m_impl{std::make_unique<Sim::SimulatorPrivate>(log, allowDebugMessages, this)}

	{
		connect(m_impl.get(), &SimulatorPrivate::projectUpdated, this, &Simulator::projectUpdated);
		connect(m_impl.get(), &SimulatorPrivate::scriptStarted, this, &Simulator::scriptStarted);
		connect(m_impl.get(), &SimulatorPrivate::scriptFinished, this, &Simulator::scriptFinished);

		return;
	}

	Simulator::~Simulator() = default;

	bool Simulator::load(QString buildPath)
	{
		return m_impl->load(buildPath);
	}

	void Simulator::clear()
	{
		return m_impl->clear();
	}

	bool Simulator::isRunning() const
	{
		return m_impl->isRunning();
	}

	bool Simulator::isPaused() const
	{
		return m_impl->isPaused();
	}

	bool Simulator::isStopped() const
	{
		return m_impl->isStopped();
	}

	bool Simulator::runScript(const SimScriptItem& script, const SimScriptItem& globalScript, qint64 timeout)
	{
		return m_impl->runScript(script, globalScript, timeout);
	}

	bool Simulator::runScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript, qint64 timeout)
	{
		return m_impl->runScripts(scripts, globalScript, timeout);
	}

	bool Simulator::stopScript()
	{
		return m_impl->stopScript();
	}

	bool Simulator::waitScript(unsigned long msecs /*= ULONG_MAX*/)
	{
		return m_impl->waitScript(msecs);
	}

	bool Simulator::scriptResult()
	{
		return m_impl->scriptResult();
	}

	bool Simulator::checkSkipOnBuildConst() const
	{
		return m_impl->checkSkipOnBuildConst();
	}

	void Simulator::setCheckSkipOnBuildConst(bool value)
	{
		return m_impl->setCheckSkipOnBuildConst(value);
	}

	ILogFile* Simulator::log()
	{
		return m_impl->log().logInterface();
	}

	bool Simulator::isLoaded() const
	{
		return m_impl->isLoaded();
	}

	QString Simulator::buildPath() const
	{
		return m_impl->buildPath();
	}

	int Simulator::buildNo() const
	{
		return m_impl->buildNo();
	}

	QString Simulator::projectName() const
	{
		return m_impl->projectName();
	}

	const Sim::Connections& Simulator::connections() const
	{
		return m_impl->connectionsPublic();
	}

	Sim::Connections& Simulator::connections()
	{
		return m_impl->connectionsPublic();
	}

	std::vector<Subsystem> Simulator::subsystems() const
	{
		auto subsystems = m_impl->subsystems();

		std::vector<Subsystem> result;
		result.reserve(subsystems.size());

		for (auto& subsystem : subsystems)
		{
			result.push_back(Subsystem{subsystem});
		}

		return result;
	}

	std::optional<LogicModule> Simulator::logicModule(QString equipmentId) const
	{
		std::optional<LogicModule> result;

		auto lm = m_impl->logicModule(equipmentId);
		if (lm != nullptr)
		{
			result = LogicModule{lm};
		}

		return result;
	}

	std::vector<LogicModule> Simulator::logicModules() const
	{
		auto lms = m_impl->logicModules();

		std::vector<LogicModule> result;
		result.reserve(lms.size());

		for (auto& lm : lms)
		{
			Q_ASSERT(lm);
			result.push_back(LogicModule{lm});
		}

		return result;
	}

	Sim::AppSignalManager& Simulator::appSignalManager()
	{
		return m_impl->appSignalManagerPublic();
	}

	const Sim::AppSignalManager& Simulator::appSignalManager() const
	{
		return m_impl->appSignalManagerPublic();
	}

	ITuningSignalManager& Simulator::tuningSignalManagerInterface()
	{
		return m_impl->tuningSignalManager();
	}

	const ITuningSignalManager& Simulator::tuningSignalManagerInterface() const
	{
		return m_impl->tuningSignalManager();
	}

	Sim::OverrideSignals& Simulator::overrideSignals()
	{
		return m_impl->overrideSignalsPublic();
	}

	const Sim::OverrideSignals& Simulator::overrideSignals() const
	{
		return m_impl->overrideSignalsPublic();
	}

	Sim::Software& Simulator::software()
	{
		return m_impl->softwarePublic();
	}

	const Sim::Software& Simulator::software() const
	{
		return m_impl->softwarePublic();
	}

	Sim::Profiles& Simulator::profiles()
	{
		return m_impl->profiles();
	}

	const Sim::Profiles& Simulator::profiles() const
	{
		return m_impl->profiles();
	}

	bool Simulator::setCurrentProfile(QString profileName)
	{
		return m_impl->setCurrentProfile(profileName);
	}

	QString Simulator::currentProfileName() const
	{
		return m_impl->currentProfileName();
	}

	const Sim::Profile& Simulator::currentProfile() const
	{
		return m_impl->currentProfile();
	}

	Sim::Control& Simulator::control()
	{
		return m_impl->controlPublic();
	}

	const Sim::Control& Simulator::control() const
	{
		return m_impl->controlPublic();
	}
} // namespace Sim
