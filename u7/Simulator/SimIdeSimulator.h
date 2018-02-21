#ifndef SIMIDESIMULATOR_H
#define SIMIDESIMULATOR_H

#include "../../Simulator/Simulator.h"
#include "../VFrame30/Schema.h"

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

private:
	VFrame30::SchemaDetailsSet m_schemaDetails;
};

#endif // SIMIDESIMULATOR_H
