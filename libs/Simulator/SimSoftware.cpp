#include <Simulator/SimSoftware.h>
#include "SimSoftwareImpl.h"

namespace Sim
{
	Software::Software(SoftwareImpl& impl) :
		m_impl{impl}
	{
	}

	QStringList Software::monitors() const
	{
		return m_impl.monitors();
	}

	std::optional<Sim::AppMonitor> Software::monitor(QString equipmentId) const
	{
		std::optional<Sim::AppMonitor> result;

		auto monitor = m_impl.monitor(equipmentId);
		if (monitor != nullptr)
		{
			result = Sim::AppMonitor{monitor};
		}

		return result;
	}

	bool Software::enabled() const
	{
		return m_impl.enabled();
	}

	void Software::setEnabled(bool value)
	{
		return m_impl.setEnabled(value);
	}

	//
	// AppMonitor
	//
	AppMonitor::AppMonitor(std::shared_ptr<AppMonitorImpl> impl) :
		m_impl{impl}
	{
		Q_ASSERT(m_impl);
	}

	AppMonitor::~AppMonitor() = default;

	QString AppMonitor::globalScript() const
	{
		return m_impl->globalScript();
	}

	const Behavior::MonitorBehavior& AppMonitor::monitorBehavior() const
	{
		return m_impl->monitorBehavior();
	}
} // namespace Sim