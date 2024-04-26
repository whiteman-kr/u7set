#include <Simulator/SimControl.h>
#include "SimControlImpl.h"

namespace Sim
{
	Control::Control(ControlImpl& impl, QObject* parent) :
		QObject{parent},
		m_impl{impl}
	{
		connect(&m_impl, &ControlImpl::stateChanged, this, &Control::stateChanged);
		connect(&m_impl, &ControlImpl::statusUpdate, this, &Control::statusUpdate);
	}

	void Control::stopThread()
	{
		m_impl.stopThread();
	}

	void Control::reset()
	{
		return m_impl.reset();
	}

	int Control::setRunList(QStringList equipmentIds)
	{
		return m_impl.setRunList(std::move(equipmentIds));
	}

	bool Control::startSimulation(std::chrono::microseconds duration)
	{
		return m_impl.startSimulation(duration);
	}

	void Control::pause()
	{
		return m_impl.pause();
	}

	void Control::stop()
	{
		return m_impl.stop();
	}

	ControlStatus Control::status() const
	{
		return ControlStatus{m_impl.controlData()};
	}

	SimControlState Control::state() const
	{
		return m_impl.state();
	}

	bool Control::isRunning() const
	{
		return m_impl.isRunning();
	}

	std::chrono::microseconds Control::duration() const
	{
		return m_impl.duration();
	}

	std::chrono::microseconds Control::leftTime() const
	{
		return m_impl.leftTime();
	}

	double Control::speedFactor() const
	{
		return m_impl.speedFactor();
	}

	void Control::setSpeedFactor(double value)
	{
		return m_impl.setSpeedFactor(value);
	}
} // namespace Sim