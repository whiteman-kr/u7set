#ifndef SIMIDESIMULATOR_H
#define SIMIDESIMULATOR_H

#include "../../Simulator/Simulator.h"
#include "../../lib/AppSignalManager.h"
#include "../../lib/Tuning/TuningSignalManager.h"
#include "../VFrame30/Schema.h"
#include "SimAppSignalManager.h"
#include "SimControlThread.h"

class SimIdeSimulator : public Sim::Simulator
{
	Q_OBJECT

public:
	SimIdeSimulator();
	virtual ~SimIdeSimulator();

public:
	bool load(QString buildPath);	// Overload from Sim::Simulator

	std::vector<VFrame30::SchemaDetails> schemasForLm(QString equipmentId) const;

protected:
	bool loadSchemaDetails(QString buildPath);
	bool loadAppSignals(QString buildPath);

public:
	SimAppSignalManager& appSignalManager();
	const SimAppSignalManager& appSignalManager() const;

	TuningSignalManager& tuningSignalManager();
	const TuningSignalManager& tuningSignalManager() const;

private:
	VFrame30::SchemaDetailsSet m_schemaDetails;

	SimAppSignalManager m_appSignalManager;
	TuningSignalManager m_tuningSignalManager;

	// Control thread
	//
	SimControlThread m_controlThread;
};

#endif // SIMIDESIMULATOR_H
