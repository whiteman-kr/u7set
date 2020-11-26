#include "SimTuningLanInterface.h"
#include "Simulator.h"

namespace Sim
{

	TuningLanInterface::TuningLanInterface(const::LanControllerInfo& lci, Lans* lans) :
		LanInterface(lci, lans)
	{
		m_log.setOutputScope("TuningLanInterface");

		setEnabled(lci.tuningEnable);

		return;
	}

	TuningLanInterface::~TuningLanInterface()
	{
	}
}
