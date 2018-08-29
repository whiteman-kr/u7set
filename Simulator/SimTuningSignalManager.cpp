#include "SimTuningSignalManager.h"

namespace Sim
{

	//
	//TuningSignalManager
	//
	TuningSignalManager::TuningSignalManager(QObject* parent) :
		::TuningSignalManager(parent),
		Output("TuningSignalManager")
	{
	}

	TuningSignalManager::~TuningSignalManager()
	{
	}

}
