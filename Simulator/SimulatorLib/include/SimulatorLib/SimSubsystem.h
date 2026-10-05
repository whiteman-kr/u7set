#pragma once

#include <SimulatorLib/SimLogicModule.h>
#include <SimulatorLib/SimVduModule.h>

#include <memory>
#include <optional>
#include <vector>


namespace Sim
{
	class LmSubsystem;
	class LmSubsystemImpl;
	class SubsystemImpl;
	class VduSubsystem;
	class VduSubsystemImpl;


	class Subsystem
	{
	public:
		explicit Subsystem(std::shared_ptr<SubsystemImpl> pimpl);
		virtual ~Subsystem();

	public:
		/// @brief Get the subsystem id.
		QString subsystemId() const;

		[[nodiscard]] bool isLmSubsystem() const;
		[[nodiscard]] bool isVduSubsystem() const;

		[[nodiscard]] LmSubsystem toLmSubsystem() const;
		[[nodiscard]] VduSubsystem toVduSubsystem() const;

	protected:
		std::shared_ptr<SubsystemImpl> m_pimpl;
	};


	class LmSubsystem : public Subsystem
	{
		friend class Simulator;
		friend class Subsystem;
		explicit LmSubsystem(std::shared_ptr<LmSubsystemImpl> pimpl);

	public:
		virtual ~LmSubsystem() override = default;

		/// @brief Get all logic modules in the subsystem.
		std::vector<Sim::LogicModule> logicModules();

		/// @brief Get a logic module by its equipment id in the subsystem.
		std::optional<Sim::LogicModule> logicModule(QString equipmentId);
	};


	class VduSubsystem : public Subsystem
	{
		friend class Simulator;
		friend class Subsystem;
		explicit VduSubsystem(std::shared_ptr<VduSubsystemImpl> pimpl);

	public:
		virtual ~VduSubsystem() override = default;

		/// @brief Get all logic modules in the subsystem.
		std::vector<Sim::VduModule> vduModules();

		/// @brief Get a logic module by its equipment id in the subsystem.
		std::optional<Sim::VduModule> vduModule(QString equipmentId);
	};
} // namespace Sim
