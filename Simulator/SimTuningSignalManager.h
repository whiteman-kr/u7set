#ifndef SIMTUNSIGNALMANGER_H
#define SIMTUNSIGNALMANGER_H

#include <unordered_map>
#include <QMutex>

#include "../lib/Tuning/TuningSignalManager.h"
#include "SimOutput.h"

namespace Sim
{

	class TuningSignalManager : public ::TuningSignalManager, protected Output
	{
		Q_OBJECT

	public:
		explicit TuningSignalManager(QObject* parent = nullptr);
		virtual ~TuningSignalManager();

	};

}

#endif // SIMTUNSIGNALMANGER_H
