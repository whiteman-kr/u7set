#pragma once

#include <SchemaClientLib/SchemaTabWidget.h>
#include "MonitorSchemaWidget.h"

namespace SchemaClientLib
{
	class MonitorSchemaWidget;
}

class MonitorCentralWidget : public SchemaClientLib::SchemaTabWidget<MonitorSchemaWidget>
{
public:
	MonitorCentralWidget(SchemaClientLib::ClientSchemaManager* schemaManager,
						 CreateSchemaWidgetFunc createSchemaWidgetFunc,
						 QWidget* parent) :
		SchemaClientLib::SchemaTabWidget<MonitorSchemaWidget>(schemaManager, createSchemaWidgetFunc, parent)
	{
	}
};
