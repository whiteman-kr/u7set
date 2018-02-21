#pragma once

#include <memory>
#include "SimulatorBasePage.h"
#include "SimLmModel.h"
#include "../../VFrame30/Schema.h"

class SimulatorSchemaPage : public SimulatorBasePage
{
	Q_OBJECT

public:
	SimulatorSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
						std::shared_ptr<SimIdeSimulator> simulator,
						QWidget* parent = nullptr);

public:
	QString schemaId() const;

private:
	std::shared_ptr<VFrame30::Schema> m_schema;
};

