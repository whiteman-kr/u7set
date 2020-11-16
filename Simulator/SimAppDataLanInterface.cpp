#include "SimAppDataLanInterface.h"
#include "Simulator.h"

namespace Sim
{

	AppDataLanInterface::AppDataLanInterface(const::LanControllerInfo& lci, Lans* lans) :
		LanInterface(lci, lans)
	{
		m_log.setOutputScope("AppDataLanInterface");
	}

	AppDataLanInterface::~AppDataLanInterface()
	{
	}

}
