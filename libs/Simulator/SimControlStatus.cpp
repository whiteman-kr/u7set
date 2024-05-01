#include <Simulator/SimControlStatus.h>
#include "SimControlImpl.h"
#include "SimLogicModuleImpl.h"

namespace Sim
{
	ControlStatus::ControlStatus(const ControlData& cd) :
		m_startTime(cd.m_startTime),
		m_currentTime(cd.m_currentTime),
		m_duration(cd.m_currentTime - cd.m_startTime),
		m_state(cd.m_state)
	{
		m_lmDeviceModes.reserve(cd.m_lms.size());

		for (const SimControlRunStruct& lm : cd.m_lms)
		{
			m_lmDeviceModes.push_back(Sim::ControlStatus::LmMode{lm.equipmentId(), lm.m_lm->deviceState()});
		}
	}
} // namespace Sim
