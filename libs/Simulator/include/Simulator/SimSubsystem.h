#pragma once

#include <memory>
#include <vector>

#include "SimLogicModule.h"

namespace Sim
{
	class LogicModule;
	class SubsystemImpl;

	class Subsystem
	{
		friend class Simulator;
		Subsystem(std::shared_ptr<SubsystemImpl> pimpl);

	public:
		~Subsystem();

	public:
		/// @brief Get the subsystem id.
		QString subsystemId() const;

		/// @brief Get all logic modules in the subsystem.
		std::vector<LogicModule> logicModules();

		/// @brief Get a logic module by its equipment id in the subsystem.
		LogicModule logicModule(QString equipmentId);

	private:
		std::shared_ptr<SubsystemImpl> m_pimpl;
	};
} // namespace Sim
