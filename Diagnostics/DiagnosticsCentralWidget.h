#pragma once

#include <SchemaClientLib/SchemaTabWidget.h>
#include "DiagSchemaWidget.h"

class DiagnosticsCentralWidget : public SchemaClientLib::SchemaTabWidget<DiagSchemaWidget>
{
public:
	DiagnosticsCentralWidget(SchemaClientLib::ClientSchemaManager* schemaManager,
							 CreateSchemaWidgetFunc createSchemaWidgetFunc,
							 QWidget* parent) :
		SchemaClientLib::SchemaTabWidget<DiagSchemaWidget>(schemaManager, createSchemaWidgetFunc, parent)
	{
	}
};
