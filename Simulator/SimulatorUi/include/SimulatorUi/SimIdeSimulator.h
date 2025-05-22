#pragma once

#include <ILogFile.h>

#include <memory>
#include <optional>
#include <set>
#include <vector>

class ITuningSignalManager;

namespace Sim
{
	class AppSignalManager;
	class Connections;
	class Control;
	class LogicModule;
	class OverrideSignals;
	class Profiles;
	class Service;
	class Simulator;
	class Software;
	class Subsystem;
	class TuningSignalManager;

	struct Profile;
} // namespace Sim

namespace Hardware
{
	class DeviceObject;
}

namespace VFrame30
{
	class SchemaDetails;
	class SchemaDetailsSet;
} // namespace VFrame30

namespace AppSignalLists
{
	class AppSignalListSet;
}

namespace SimUi
{

	class SimIdeSimulator : public QObject
	{
		Q_OBJECT

	public:
		SimIdeSimulator(ILogFile* log, bool allowDebugMessages, QObject* parent);

		SimIdeSimulator() = delete;
		SimIdeSimulator(const SimIdeSimulator&) = delete;
		SimIdeSimulator(SimIdeSimulator&&) = delete;
		virtual ~SimIdeSimulator();

		SimIdeSimulator& operator=(const SimIdeSimulator&) = delete;
		SimIdeSimulator& operator=(SimIdeSimulator&&) = delete;

	public:
		bool load(QString buildPath); // Overload from Sim::Simulator
		void clear();                 // Overload from Sim::Simulator

		const VFrame30::SchemaDetailsSet& schemaDetails() const;
		std::vector<VFrame30::SchemaDetails> schemasForLm(QString equipmentId) const;

		std::shared_ptr<Hardware::DeviceObject> monitorEquipment() const;

		// Form the Sim::Simulator
		//
	public:
		// Flow control
		//
		[[nodiscard]] bool isRunning() const;
		[[nodiscard]] bool isPaused() const;
		[[nodiscard]] bool isStopped() const;

		[[nodiscard]] ILogFile* log();

		[[nodiscard]] bool isLoaded() const;
		[[nodiscard]] QString buildPath() const;
		[[nodiscard]] int buildNo() const;

		[[nodiscard]] QString projectName() const;

		[[nodiscard]] const Sim::Connections& connections() const;
		[[nodiscard]] Sim::Connections& connections();

		[[nodiscard]] std::vector<Sim::Subsystem> subsystems() const;

		[[nodiscard]] std::optional<Sim::LogicModule> logicModule(QString equipmentId) const;
		[[nodiscard]] std::vector<Sim::LogicModule> logicModules() const;

		[[nodiscard]] Sim::AppSignalManager& appSignalManager();
		[[nodiscard]] const Sim::AppSignalManager& appSignalManager() const;

		[[nodiscard]] ITuningSignalManager& tuningSignalManagerInterface();
		[[nodiscard]] const ITuningSignalManager& tuningSignalManagerInterface() const;

		[[nodiscard]] Sim::OverrideSignals& overrideSignals();
		[[nodiscard]] const Sim::OverrideSignals& overrideSignals() const;

		[[nodiscard]] Sim::Software& software();
		[[nodiscard]] const Sim::Software& software() const;

		[[nodiscard]] Sim::Service& service();
		[[nodiscard]] const Sim::Service& service() const;

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
		bool loadAppSignalLists(QString buildPath);

	public:
		const Sim::Simulator* simulator() const;
		Sim::Simulator* simulator();

		std::vector<VFrame30::SchemaDetails> schemasDetails() const;
		std::set<QString> schemaAppSignals(const QString& schemaId);

		QStringList schemasByAppSignalId(const QString& appSignalId) const;
		QStringList schemasByLoopbackId(const QString& loopbackId) const;

		const AppSignalLists::AppSignalListSet& appSignalListSet() const;
		AppSignalLists::AppSignalListSet& appSignalListSet();

	private:
		std::unique_ptr<Sim::Simulator> m_simulator;

		std::unique_ptr<VFrame30::SchemaDetailsSet> m_schemaDetails;
		std::unique_ptr<AppSignalLists::AppSignalListSet> m_appSignalListSet;

		// The monitor equipment is a filtered tree of all devices (excluding signals),
		// which is read from the file 'Common/MonitorEquipment.dat.
		//
		std::shared_ptr<Hardware::DeviceObject> m_monitorEquipment;
	};

} // namespace SimUi
