#pragma once

#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"

class SimIdeSimulator;

class SimSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit SimSchemaManager(SimIdeSimulator* simulator, QObject* parent = nullptr);

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

	// Slots
	//
protected slots:
	void slot_projectUpdated();

	// Data
	//
private:
	SimIdeSimulator* m_simulator = nullptr;
};

