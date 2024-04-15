#pragma once
#include <Behavior/MonitorBehavior.h>

namespace Sim
{
	class SoftwareImpl;
	class AppMonitor;
	class AppMonitorImpl;

	//
	// Software
	//
	class Software
	{
	private:
		friend class SimulatorPrivate;
		explicit Software(SoftwareImpl& impl);

	public:
		// Monitor
		//
		QStringList monitors() const; // Returns EquipmentIDs of all Monitors
		std::optional<Sim::AppMonitor> monitor(QString equipmentId) const;

	public:
		[[nodiscard]] bool enabled() const;
		void setEnabled(bool value);

	private:
		SoftwareImpl& m_impl;
	};

	//
	// AppMonitor
	//
	class AppMonitor
	{
	public:
		AppMonitor(std::shared_ptr<AppMonitorImpl> impl);
		~AppMonitor();

	public:
		QString globalScript() const;
		const Behavior::MonitorBehavior& monitorBehavior() const;

	private:
		std::shared_ptr<AppMonitorImpl> m_impl;
	};
} // namespace Sim
