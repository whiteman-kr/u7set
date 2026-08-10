#include <SimulatorLib/SimSubsystem.h>

#include <SimulatorLib/SimLogicModule.h>

#include "./Modules/SimLogicModuleImpl.h"
#include "SimLmSubsystemImpl.h"
#include "SimSubsystemImpl.h"


namespace Sim
{
	//
	// Subsystem
	//
	Subsystem::Subsystem(std::shared_ptr<SubsystemImpl> pimpl) :
		m_pimpl(pimpl)
	{
		Q_ASSERT(m_pimpl);
	}

	Subsystem::~Subsystem() = default;

	QString Subsystem::subsystemId() const
	{
		return m_pimpl->subsystemId();
	}

	bool Subsystem::isLmSubsystem() const
	{
		return m_pimpl->isLmSubsystem();
	}

	bool Subsystem::isVduSubsystem() const
	{
		return m_pimpl->isVduSubsystem();
	}

	const LmSubsystem& Subsystem::toLmSubsystem() const
	{
		assert(isLmSubsystem() == true);
		return dynamic_cast<const LmSubsystem&>(*this);
	}

	//
	// LmSubsystem
	//
	LmSubsystem::LmSubsystem(std::shared_ptr<LmSubsystemImpl> pimpl) :
		Subsystem{pimpl}
	{
		Q_ASSERT(m_pimpl);
		Q_ASSERT(m_pimpl->isLmSubsystem() == true);
	}

	std::vector<LogicModule> LmSubsystem::logicModules()
	{
		std::vector<LogicModule> result;

		if (m_pimpl->isLmSubsystem() == false)
		{
			return result;
		}

		auto lms = m_pimpl->toLmSubsystem()->logicModules();

		result.reserve(lms.size());
		std::transform(lms.begin(),
					   lms.end(),
					   std::back_inserter(result),
					   [](const std::shared_ptr<LogicModuleImpl>& lmImpl)
					   {
						   return Sim::LogicModule{lmImpl};
					   });

		return result;
	}

	Sim::LogicModule LmSubsystem::logicModule(QString equipmentId)
	{
		std::shared_ptr<LogicModuleImpl> lm =
			m_pimpl->isLmSubsystem() == true ? m_pimpl->toLmSubsystem()->logicModule(equipmentId) : nullptr;

		return Sim::LogicModule{lm};
	}

} // namespace Sim