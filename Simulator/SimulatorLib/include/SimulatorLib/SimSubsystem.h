#pragma once

#include <memory>
#include <vector>


namespace Sim
{
	class LmSubsystem;
	class LmSubsystemImpl;
	class LogicModule;
	class SubsystemImpl;


	class Subsystem
	{
	protected:
		explicit Subsystem(std::shared_ptr<SubsystemImpl> pimpl);

	public:
		virtual ~Subsystem();

	public:
		/// @brief Get the subsystem id.
		QString subsystemId() const;

		[[nodiscard]] bool isLmSubsystem() const;
		[[nodiscard]] bool isVduSubsystem() const;

		[[nodiscard]] const LmSubsystem& toLmSubsystem() const;

	protected:
		std::shared_ptr<SubsystemImpl> m_pimpl;
	};


	class LmSubsystem : public Subsystem
	{
		friend class Simulator;
		explicit LmSubsystem(std::shared_ptr<LmSubsystemImpl> pimpl);

	public:
		virtual ~LmSubsystem() override = default;

		/// @brief Get all logic modules in the subsystem.
		std::vector<Sim::LogicModule> logicModules();

		/// @brief Get a logic module by its equipment id in the subsystem.
		Sim::LogicModule logicModule(QString equipmentId);
	};
} // namespace Sim
