#include "TuningSchemaView.h"
#include "TuningSchemaWidget.h"
#include "MainWindow.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/MonitorSchema.h"

TuningSchemaView::TuningSchemaView(TuningSchemaManager* schemaManager, QWidget* parent /*= nullptr*/)
	:  VFrame30::ClientSchemaView(schemaManager, parent)
{
}

TuningSchemaView::~TuningSchemaView()
{
}

