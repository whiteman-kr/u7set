#pragma once

#include "../../Simulator/SimLogicModule.h"
#include "../../VFrame30/Schema.h"
#include "../../VFrame30/AppSignalController.h"
#include "../../VFrame30/TuningController.h"
#include "SimBasePage.h"
#include "SimSchemaWidget.h"

class SimSchemaPage : public SimBasePage
{
	Q_OBJECT

public:
	SimSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
				  SimIdeSimulator* simulator,
				  SimSchemaManager* schemaManager,
				  VFrame30::AppSignalController* appSignalController,
				  QWidget* parent = nullptr);

public:
	QString schemaId() const;

	const QStringList& highlightIds() const;
	void setHighlightIds(const QStringList& value);

protected slots:
	void controlStateChanged(Sim::SimControlState state);

public:
	SimSchemaWidget* simSchemaWidget();
	const SimSchemaWidget* simSchemaWidget() const;

private:
	SimSchemaWidget* m_schemaWidget = nullptr;
};

