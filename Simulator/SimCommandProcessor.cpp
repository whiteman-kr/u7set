#include <cassert>
#include "SimCommandProcessor.h"
#include "SimCommandProcessor_LM_SF00.h"

namespace Sim
{
	const std::map<QString, std::function<CommandProcessor*()>> CommandProcessor::m_lmToFactory
		{
			{"LM-SF00", [](){	return new CommandProcessor_LM_SF00;	}},
			//{"LM-SR02", [](){	return new CommandProcessor_LM_SR02;	}},
			//{"LM-SR01", [](){	return new CommandProcessor_LM_SR01;	}},
		};

	CommandProcessor::CommandProcessor()
	{
	}

	CommandProcessor::~CommandProcessor()
	{
	}

	CommandProcessor* CommandProcessor::createInstance(QString logicModuleName, QString equipmentId)
	{
		if (auto it = m_lmToFactory.find(logicModuleName);
			it == m_lmToFactory.end())
		{
			return nullptr;
		}
		else
		{
			CommandProcessor* result = it->second();
			assert(result);

			result->setOutputScope(equipmentId);

			return result;
		}
	}
}
