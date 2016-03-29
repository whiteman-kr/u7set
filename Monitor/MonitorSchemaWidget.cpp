#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"


//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Scheme> scheme) :
	BaseSchemeWidget(scheme, new MonitorSchemaView())
{
	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
}

void MonitorSchemaWidget::createActions()
{

}