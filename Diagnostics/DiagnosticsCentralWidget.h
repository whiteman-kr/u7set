#pragma once

#include "DiagSchemaView.h"
#include "DiagSchemaWidget.h"

#include <SchemaClientLib/SchemaTabWidget.h>

class DiagnosticsCentralWidget : public SchemaClientLib::SchemaTabWidget<DiagSchemaWidget>
{
public:
	DiagnosticsCentralWidget(SchemaClientLib::ClientSchemaManager* schemaManager,
							 CreateSchemaWidgetFunc createSchemaWidgetFunc,
							 QWidget* parent) :
		SchemaClientLib::SchemaTabWidget<DiagSchemaWidget>(schemaManager, createSchemaWidgetFunc, parent)
	{
	}


	void updateConfiguration(const DiagConfigSettings& configuration)
	{
		setStartSchemaId(configuration.startSchemaId);

		// Update all tabs with new configuration.
		// Do not do it by signal/slot, as MainWindow calls this method directly.
		//
		for (int i = 0; i < count(); i++)
		{
			auto schemaWidget = dynamic_cast<DiagSchemaWidget*>(widget(i));

			if (schemaWidget != nullptr)
			{
				schemaWidget->diagSchemaView()->updateConfiguration(configuration);
			}
		}

		// updateConfiguration resets schema, which triggers after create scripts, which can require GlobalScript.
		// At this point GlobalScript is considered evaluated.
		//
		auto diagSchemaManager = dynamic_cast<DiagnosticsSchemaManager*>(m_schemaManager);
		Q_ASSERT(diagSchemaManager);

		diagSchemaManager->updateConfiguration(configuration);

		return;
	}
};
