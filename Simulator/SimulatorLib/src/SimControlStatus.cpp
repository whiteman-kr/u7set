#include <SimulatorLib/SimControlStatus.h>

#include "./Modules/SimLogicModuleImpl.h"
#include "SimControlImpl.h"

namespace Sim
{
	ControlStatus::ControlStatus(const ControlData& cd) :
		m_startTime(cd.m_startTime),
		m_currentTime(cd.m_currentTime),
		m_duration(cd.m_currentTime - cd.m_startTime),
		m_state(cd.m_state)
	{
		m_lmDeviceModes.reserve(cd.m_modules.size());

		for (const SimControlRunStruct& lm : cd.m_modules)
		{
			m_lmDeviceModes.push_back(Sim::ControlStatus::LmMode{lm.equipmentId(), lm->deviceState()});
		}
	}
} // namespace Sim
