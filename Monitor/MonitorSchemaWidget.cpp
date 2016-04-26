#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"
#include "SchemaManager.h"

//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaManager* schemaManager) :
	BaseSchemaWidget(schema, new MonitorSchemaView(schemaManager)),
	m_schemaManager(schemaManager)
{
	assert(m_schemaManager);

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
