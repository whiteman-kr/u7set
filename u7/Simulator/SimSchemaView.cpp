#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/PropertyNames.h"


// MonitorView
//
SimSchemaView::SimSchemaView(SimSchemaManager* schemaManager, QWidget *parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
	qDebug() << Q_FUNC_INFO;
	return;
}

SimSchemaView::~SimSchemaView()
{
	qDebug() << Q_FUNC_INFO;
	return;
}




