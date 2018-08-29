#include "MainWindow.h"
#include "TuningSchemaWidget.h"
#include "../VFrame30/MonitorSchema.h"



TuningSchemaWidget::TuningSchemaWidget(TuningSignalManager* tuningSignalManager,
									   VFrame30::TuningController* tuningController,
									   std::shared_ptr<VFrame30::Schema> schema,
									   TuningSchemaManager* schemaManager) :
	VFrame30::ClientSchemaWidget(new TuningSchemaView(schemaManager), schema, schemaManager)
{
	assert(tuningSignalManager);
	assert(tuningController);
	assert(schemaManager);

	clientSchemaView()->setTuningController(tuningController);

	// --
	//
	connect(clientSchemaView(), &VFrame30::ClientSchemaView::signal_setSchema, this, &VFrame30::ClientSchemaWidget::setSchema);

	return;
}

TuningSchemaWidget::~TuningSchemaWidget()
{
}

