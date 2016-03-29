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
