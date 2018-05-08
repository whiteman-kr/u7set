#include "SimCommandProcessor.h"
#include "SimCommandProcessor_LM_SF00.h"

namespace Sim
{
	const std::map<QString, std::function<CommandProcessor*()>> CommandProcessor::m_lmToFactory
		{
			{"LM-SF00", []{	return new CommandProcessor_LM_SF00();	}},
			//{"LM-SR01", []{	return new CommandProcessor_LM_SR01();	}},
			//{"LM-SR02", []{	return new CommandProcessor_LM_SR02();	}},
		};

	CommandProcessor::CommandProcessor()
	{
	}

	CommandProcessor::~CommandProcessor()
	{
	}

	CommandProcessor* CommandProcessor::createInstance(QString logicModuleName)
	{
		return nullptr;
	}
}
