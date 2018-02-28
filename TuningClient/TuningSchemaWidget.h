#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/SchemaView.h"
#include "TuningSchemaView.h"
#include "TuningSchemaManager.h"

class TuningSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

	TuningSchemaWidget() = delete;
public:

	TuningSchemaWidget(TuningSignalManager* tuningSignalManager,
					   VFrame30::TuningController* tuningController,
					   std::shared_ptr<VFrame30::Schema> schema,
					   TuningSchemaManager* schemaManager);
	~TuningSchemaWidget();

	//TuningSchemaView* tuningSchemaView();

private:
};

#endif // TUNINGSCHEMAWIDGET_H
