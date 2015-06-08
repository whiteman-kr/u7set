#include "WorkflowSchemeView.h"

WorkflowSchemeView::WorkflowSchemeView(QWidget *parent)
	: SchemeView(parent)
{
	qDebug() << Q_FUNC_INFO;
}

WorkflowSchemeView::~WorkflowSchemeView()
{
	qDebug() << Q_FUNC_INFO;
}

void WorkflowSchemeView::paintEvent(QPaintEvent* pe)
{
	// Draw videoframe
	//
	VFrame30::SchemeView::paintEvent(pe);
}
