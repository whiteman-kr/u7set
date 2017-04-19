#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include "../VFrame30/BaseSchemaWidget.h"
#include "../VFrame30/SchemaView.h"
#include "TuningSchemaView.h"
#include "SchemaStorage.h"

class TuningSchemaWidget : public VFrame30::BaseSchemaWidget
{
	Q_OBJECT

	TuningSchemaWidget() = delete;
public:

	TuningSchemaWidget(TuningObjectManager *tuningObjectManager, std::shared_ptr<VFrame30::Schema> schema, SchemaStorage* schemaStorage, const QString &globalScript);
	~TuningSchemaWidget();

	bool slot_setSchema(QString schemaId);

	TuningSchemaView* tuningSchemaView();

private:
	SchemaStorage* m_schemaStorage = nullptr;

};

#endif // TUNINGSCHEMAWIDGET_H
