#pragma once

namespace Sim
{
	class SimulatorPrivate;
	class ServiceImpl;

	class Service final
	{
	public:
		explicit Service(SimulatorPrivate& simulator);
		~Service();

	public:
		[[nodiscard]] bool enabled() const;
		void setEnabled(bool enable);

	private:
		SimulatorPrivate& m_simulator;
		std::unique_ptr<ServiceImpl> m_impl;
	};
} // namespace Sim
