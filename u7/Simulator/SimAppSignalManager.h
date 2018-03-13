#ifndef SIMAPPSIGNALMANAGER_H
#define SIMAPPSIGNALMANAGER_H

#include <memory>
#include <map>
#include "../../lib/Signal.h"
#include "../../lib/AppSignalManager.h"
#include "../Simulator/SimOutput.h"

class SimIdeSimulator;

class SimAppSignalManager : public AppSignalManager, protected Sim::Output
{
	Q_OBJECT

public:
	explicit SimAppSignalManager(SimIdeSimulator* simulator,
								 QObject* parent = nullptr);
	virtual ~SimAppSignalManager();

public:
	bool load(QString fileName);

private:
	SimIdeSimulator* m_simulator = nullptr;

	std::map<QString, Signal> m_signals;	// Except AppSignalParam, we need Signal as it has more information (like offset in memory)
};

#endif // SIMAPPSIGNALMANAGER_H
