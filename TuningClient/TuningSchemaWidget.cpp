#include "TuningSchemaWidget.h"
#include "../VFrame30/MonitorSchema.h"


TuningSchemaWidget::TuningSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaStorage* schemaStorage) :
	BaseSchemaWidget(schema, new TuningSchemaView(schemaStorage)),
	m_schemaStorage(schemaStorage)
{
	assert(m_schemaStorage);

	// --
	//
	/*connect(schemaView(), &VFrame30::SchemaView::signal_schemaChanged, this, [this](VFrame30::Schema* schema)
		{
			emit this->signal_schemaChanged(this, schema);
		});*/

	// --
	//
	connect(tuningSchemaView(), &TuningSchemaView::signal_setSchema, this, &TuningSchemaWidget::slot_setSchema);
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


	// We can't change schema here, because we need to save history, so emit signal and change schema
	// in MonitorSchemaWidget
	//
	//emit signal_setSchema(schemaId);

//	assert(m_schemaManager);

//	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager->schema(schemaId);

//	if (schema == nullptr)
//	{
//		return false;
//	}

//	// --
//	//
////	if (m_historyIterator != m_history.end())
////	{
////		m_history.erase(m_historyIterator, m_history.end());
////	}

////	SchemaHistoryItem hi;
////	hi.schemaId = schemaId;
////	hi.zoom = 100.0;

////	m_history.push_back(hi);
////	m_historyIterator = m_history.end();

//	// --
//	//
//	SchemaView::setSchema(schema, false);
//	setZoom(100.0, true);

	return true;
}



TuningSchemaView* TuningSchemaWidget::tuningSchemaView()
{
	TuningSchemaView* result = dynamic_cast<TuningSchemaView*>(schemaView());
	assert(result);
	return result;
}
