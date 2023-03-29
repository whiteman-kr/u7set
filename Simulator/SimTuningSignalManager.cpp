#include "SimTuningSignalManager.h"

namespace Sim
{

	//
	// TuningSignalManager
	//
	TuningSignalManager::TuningSignalManager(ScopedLog log, QObject* parent) :
		::TuningSignalManager({}, log.logInterface(), parent),
		m_log(log, "TuningSignalManager")
	{
	}
}
