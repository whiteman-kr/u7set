#ifndef SIMTUNSIGNALMANGER_H
#define SIMTUNSIGNALMANGER_H

#include "../lib/Tuning/TuningSignalManager.h"
#include "SimScopedLog.h"

namespace Sim
{

	class TuningSignalManager : public ::TuningSignalManager
	{
		Q_OBJECT

	public:
		explicit TuningSignalManager(ScopedLog log, QObject* parent = nullptr);
		virtual ~TuningSignalManager() = default;

	private:
		ScopedLog m_log;
	};

}

#endif // SIMTUNSIGNALMANGER_H
