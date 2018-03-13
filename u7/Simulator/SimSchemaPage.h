#pragma once

#include <memory>
#include "SimBasePage.h"
#include "SimLmModel.h"
#include "SimSchemaWidget.h"
#include "../../VFrame30/Schema.h"
#include "../../VFrame30/AppSignalController.h"
#include "../../VFrame30/TuningController.h"

class SimSchemaPage : public SimBasePage
{
	Q_OBJECT

public:
	SimSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
				  SimIdeSimulator* simulator,
				  SimSchemaManager* schemaManager,
				  VFrame30::AppSignalController* appSignalController,
				  VFrame30::TuningController* tuningController,
				  QWidget* parent = nullptr);

public:
	QString schemaId() const;

private:
	SimSchemaWidget* m_schemaWidget = nullptr;
};

