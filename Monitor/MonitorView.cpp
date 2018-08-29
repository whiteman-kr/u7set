#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"


// MonitorView
//
MonitorView::MonitorView(MonitorSchemaManager* schemaManager, QWidget *parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
	qDebug() << Q_FUNC_INFO;
	return;
}

MonitorView::~MonitorView()
{
	qDebug() << Q_FUNC_INFO;
	return;
}




