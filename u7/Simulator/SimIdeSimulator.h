#pragma once

#include "../../Simulator/SimScopedLog.h"
#include "../VFrame30/SchemaDetails.h"

namespace Sim
{
	class AppSignalManager;
	class Connections;
	class Control;
	class LogicModule;
	class OverrideSignals;
	class Profiles;
	struct Profile;
	class Simulator;
	class Software;
	class Subsystem;
	class TuningSignalManager;
}

class SimIdeSimulator : public QObject
{
	Q_OBJECT

public:
	SimIdeSimulator(ILogFile* log, bool allowDebugMessages, QObject* parent);
	virtual ~SimIdeSimulator();

public:
	bool load(QString buildPath); // Overload from Sim::Simulator
	void clear();                 // Overload from Sim::Simulator

	const VFrame30::SchemaDetailsSet& schemaDetails() const;
	std::vector<VFrame30::SchemaDetails> schemasForLm(QString equipmentId) const;

	// Form the Sim::Simulator
	//
public:
	// Flow control
	//
	[[nodiscard]] bool isRunning() const;
	[[nodiscard]] bool isPaused() const;
	[[nodiscard]] bool isStopped() const;

	[[nodiscard]] Sim::ScopedLog& log();

	[[nodiscard]] bool isLoaded() const;
	[[nodiscard]] QString buildPath() const;
	[[nodiscard]] int buildNo() const;

	[[nodiscard]] QString projectName() const;

	[[nodiscard]] const Sim::Connections& connections() const;
	[[nodiscard]] Sim::Connections& connections();

	[[nodiscard]] std::vector<std::shared_ptr<Sim::Subsystem>> subsystems() const;
	[[nodiscard]] std::shared_ptr<Sim::LogicModule> logicModule(QString equipmentId) const;
	[[nodiscard]] std::vector<std::shared_ptr<Sim::LogicModule>> logicModules() const;

	[[nodiscard]] Sim::AppSignalManager& appSignalManager();
	[[nodiscard]] const Sim::AppSignalManager& appSignalManager() const;

	[[nodiscard]] Sim::TuningSignalManager& tuningSignalManager();
	[[nodiscard]] const Sim::TuningSignalManager& tuningSignalManager() const;

	[[nodiscard]] Sim::OverrideSignals& overrideSignals();
	[[nodiscard]] const Sim::OverrideSignals& overrideSignals() const;

	[[nodiscard]] Sim::Software& software();
	[[nodiscard]] const Sim::Software& software() const;

	[[nodiscard]] Sim::Profiles& profiles();
	[[nodiscard]] const Sim::Profiles& profiles() const;

	bool setCurrentProfile(QString profileName);
	[[nodiscard]] QString currentProfileName() const;
	[[nodiscard]] const Sim::Profile& currentProfile() const;

	[[nodiscard]] Sim::Control& control();
	[[nodiscard]] const Sim::Control& control() const;

signals:
	void schemaDetailsUpdated();

	// Sim::Simulator has signal projectUpdated() too, but we need to override it for SimIdeSimulator, as in load(...)
	// there is restoring of tuning/arming key, and we need to receive this signal AFTER these keys were restored.
	//
	void projectUpdated();

protected:
	bool loadSchemaDetails(QString buildPath);

public:
	const Sim::Simulator* simulator() const;
	Sim::Simulator* simulator();

	std::vector<VFrame30::SchemaDetails> schemasDetails() const;
	std::set<QString> schemaAppSignals(const QString& schemaId);

	QStringList schemasByAppSignalId(const QString& appSignalId) const;
	QStringList schemasByLoopbackId(const QString& loopbackId) const;

private:
	std::unique_ptr<Sim::Simulator> m_simulator;
	VFrame30::SchemaDetailsSet m_schemaDetails;
};

