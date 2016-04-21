#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"


//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema) :
	BaseSchemaWidget(schema, new MonitorSchemaView())
{
	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
}

void MonitorSchemaWidget::createActions()
{

}

QString MonitorSchemaWidget::schemaId() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->schemaID();
}

QString MonitorSchemaWidget::caption() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->caption();
}
