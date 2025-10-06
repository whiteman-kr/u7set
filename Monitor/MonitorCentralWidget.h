#pragma once

#include "MonitorSchemaView.h"
#include "MonitorSchemaWidget.h"
#include <SchemaClientLib/SchemaTabWidget.h>


struct MonitorConfigSettings;


class MonitorCentralWidget : public SchemaClientLib::SchemaTabWidget<MonitorSchemaWidget>
{
public:
	MonitorCentralWidget(SchemaClientLib::ClientSchemaManager* schemaManager,
						 CreateSchemaWidgetFunc createSchemaWidgetFunc,
						 QWidget* parent) :
		SchemaClientLib::SchemaTabWidget<MonitorSchemaWidget>(schemaManager, createSchemaWidgetFunc, parent)
	{
	}

	void updateConfiguration(const MonitorConfigSettings& configuration)
	{
		setStartSchemaId(configuration.startSchemaId);

		// Update all tabs with new configuration.
		// Do not do it by signal/slot, as MainWindow calls this method directly.
		//
		for (int i = 0; i < count(); i++)
		{
			auto schemaWidget = dynamic_cast<MonitorSchemaWidget*>(widget(i));

			if (schemaWidget != nullptr)
			{
				schemaWidget->monitorSchemaView()->updateConfiguration(configuration);
			}
		}

		// updateConfiguration resets schema, which triggers after create scripts, which can require GlobalScript.
		// At this point GlobalScript is considered evaluated.
		//
		auto monitorSchemaManager = dynamic_cast<MonitorSchemaManager*>(m_schemaManager);
		Q_ASSERT(monitorSchemaManager);

		monitorSchemaManager->updateConfiguration(configuration);

		return;
	}
};
