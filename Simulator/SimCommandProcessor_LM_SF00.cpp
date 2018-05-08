#include "SimCommandProcessor_LM_SF00.h"

namespace Sim
{

	CommandProcessor_LM_SF00::CommandProcessor_LM_SF00() :
		CommandProcessor()
	{

	}

	QString CommandProcessor_LM_SF00::logicModuleName() const
	{
		return m_logicModuleName;
	}

}
