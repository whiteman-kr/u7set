#ifndef SIMTUNSIGNALMANGER_H
#define SIMTUNSIGNALMANGER_H

#include <unordered_map>
#include <QMutex>

#include "../lib/Tuning/TuningSignalManager.h"
#include "SimScopedLog.h"

namespace Sim
{

	class TuningSignalManager : public ::TuningSignalManager
	{
		Q_OBJECT

	public:
		explicit TuningSignalManager(ScopedLog log, QObject* parent = nullptr);
		virtual ~TuningSignalManager();

	private:
		ScopedLog m_log;
	};

}

#endif // SIMTUNSIGNALMANGER_H
