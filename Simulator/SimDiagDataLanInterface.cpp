#include "SimDiagDataLanInterface.h"
#include "Simulator.h"

namespace Sim
{

	DiagDataLanInterface::DiagDataLanInterface(const::LanControllerInfo& lci, Lans* lans) :
		LanInterface(lci, lans)
	{
		m_log.setOutputScope("DiagDataLanInterface");

		setEnabled(lci.diagDataEnable);

		return;
	}

	DiagDataLanInterface::~DiagDataLanInterface()
	{
	}

}
