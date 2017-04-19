#include "MainWindow.h"
#include "TuningSchemaWidget.h"
#include "../VFrame30/MonitorSchema.h"



TuningSchemaWidget::TuningSchemaWidget(TuningObjectManager *tuningObjectManager, std::shared_ptr<VFrame30::Schema> schema, SchemaStorage* schemaStorage, const QString& globalScript) :
	BaseSchemaWidget(schema, new TuningSchemaView(schemaStorage, globalScript)),
	m_schemaStorage(schemaStorage)
{
	assert(tuningObjectManager);
	assert(m_schemaStorage);

	// --
	//
	connect(tuningSchemaView(), &TuningSchemaView::signal_setSchema, this, &TuningSchemaWidget::slot_setSchema);

	tuningObjectManager->connectTuningController(&tuningSchemaView()->tuningController());

}

TuningSchemaWidget::~TuningSchemaWidget()
{

}

bool TuningSchemaWidget::slot_setSchema(QString schemaId)
{
	std::shared_ptr<VFrame30::Schema> schema = m_schemaStorage->schema(schemaId);

	if (schema == nullptr)
	{
		// and there is no startSchemaId (((
		// Just create an empty schema
		//
		schema = std::make_shared<VFrame30::MonitorSchema>();
		schema->setSchemaId("EMPTYSCHEMA");
		schema->setCaption("Empty Schema");
	}

	// --
	//
	setSchema(schema, false);
	setZoom(100.0, true);

	return true;
}



TuningSchemaView* TuningSchemaWidget::tuningSchemaView()
{
	TuningSchemaView* result = dynamic_cast<TuningSchemaView*>(schemaView());
	assert(result);
	return result;
}
