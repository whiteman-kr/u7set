#include "WorkflowSchemeWidget.h"
#include "WorkflowSchemeView.h"


//
//
//	WorkflowSchemeWidget
//
//

WorkflowSchemeWidget::WorkflowSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme) :
	BaseSchemeWidget(scheme, new WorkflowSchemeView())
{
	return;
}

WorkflowSchemeWidget::~WorkflowSchemeWidget()
{
}

void WorkflowSchemeWidget::createActions()
{

}
