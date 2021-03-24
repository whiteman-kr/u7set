#include "SimTuningSignalManager.h"

namespace Sim
{

	//
	// TuningSignalManager
	//
	TuningSignalManager::TuningSignalManager(ScopedLog log, QObject* parent) :
		::TuningSignalManager(parent),
		m_log(log, "TuningSignalManager")
	{
	}

	TuningSignalManager::~TuningSignalManager()
	{
	}

}
