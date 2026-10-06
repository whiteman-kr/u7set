#include <SimulatorLib/SimSubsystem.h>

#include <SimulatorLib/SimLogicModule.h>
#include <SimulatorLib/SimVduModule.h>

#include "../Modules/SimLogicModuleImpl.h"
#include "SimLmSubsystemImpl.h"
#include "SimSubsystemImpl.h"
#include "SimVduSubsystemImpl.h"


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

	LmSubsystem Subsystem::toLmSubsystem() const
	{
		assert(isLmSubsystem() == true);

		auto lmss = std::dynamic_pointer_cast<LmSubsystemImpl>(m_pimpl);
		assert(lmss);
		return LmSubsystem{lmss};
	}

	VduSubsystem Subsystem::toVduSubsystem() const
	{
		assert(isVduSubsystem() == true);

		auto vduss = std::dynamic_pointer_cast<VduSubsystemImpl>(m_pimpl);
		assert(vduss);
		return VduSubsystem{vduss};
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

	std::optional<Sim::LogicModule> LmSubsystem::logicModule(QString equipmentId)
	{
		std::shared_ptr<LogicModuleImpl> lm =
			m_pimpl->isLmSubsystem() == true ? m_pimpl->toLmSubsystem()->logicModule(equipmentId) : nullptr;

		return lm ? std::optional<Sim::LogicModule>{std::in_place, lm} : std::nullopt;
	}

	//
	// VduSubsystem
	//
	VduSubsystem::VduSubsystem(std::shared_ptr<VduSubsystemImpl> pimpl) :
		Subsystem{pimpl}
	{
		Q_ASSERT(m_pimpl);
		Q_ASSERT(m_pimpl->isVduSubsystem() == true);
	}

	std::vector<VduModule> VduSubsystem::vduModules()
	{
		std::vector<VduModule> result;

		if (m_pimpl->isVduSubsystem() == false)
		{
			return result;
		}

		auto vdus = m_pimpl->toVduSubsystem()->vduModules();

		result.reserve(vdus.size());
		std::transform(vdus.begin(),
					   vdus.end(),
					   std::back_inserter(result),
					   [](const std::shared_ptr<VduModuleImpl>& vduImpl)
					   {
						   return Sim::VduModule{vduImpl};
					   });

		return result;
	}

	std::optional<Sim::VduModule> VduSubsystem::vduModule(QString equipmentId)
	{
		std::shared_ptr<VduModuleImpl> vdu =
			m_pimpl->isVduSubsystem() == true ? m_pimpl->toVduSubsystem()->vduModule(equipmentId) : nullptr;

		return vdu ? std::optional<Sim::VduModule>{std::in_place, vdu} : std::nullopt;
	}
} // namespace Sim