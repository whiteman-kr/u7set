#include <Simulator/SimSubsystem.h>
#include "SimLogicModuleImpl.h"
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

	std::vector<LogicModule> Subsystem::logicModules()
	{
		auto lmImpl = m_pimpl->logicModules();

		std::vector<LogicModule> result;
		result.reserve(lmImpl.size());

		for (auto& lm : lmImpl)
		{
			result.push_back(LogicModule{lm});
		}

		return result;
	}

	LogicModule Subsystem::logicModule(QString equipmentId)
	{
		return LogicModule{m_pimpl->logicModule(equipmentId)};
	}

} // namespace Sim