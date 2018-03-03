#pragma once

#include <memory>
#include "SimulatorBasePage.h"
#include "SimLmModel.h"
#include "SimSchemaWidget.h"
#include "../../VFrame30/Schema.h"
#include "../../VFrame30/AppSignalController.h"
#include "../../VFrame30/TuningController.h"

class SimulatorSchemaPage : public SimulatorBasePage
{
	Q_OBJECT

public:
	SimulatorSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
						std::shared_ptr<SimIdeSimulator> simulator,
						SimSchemaManager* schemaManager,
						VFrame30::AppSignalController* appSignalController,
						VFrame30::TuningController* tuningController,
						QWidget* parent = nullptr);

public:
	QString schemaId() const;

private:
	SimSchemaWidget* m_schemaWidget = nullptr;
};

