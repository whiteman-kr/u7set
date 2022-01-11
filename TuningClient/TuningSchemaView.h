#ifndef TUNINGSCHEMAVIEW_H
#define TUNINGSCHEMAVIEW_H

#include "TuningSchemaManager.h"
#include "../VFrame30/ClientSchemaView.h"
#include "../lib/Tuning/TuningSignalManager.h"

class TuningSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	TuningSchemaView(TuningSchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~TuningSchemaView() = default;

protected:
	virtual void updateScriptGlobalVars(QJSEngine& engine) override;
};

#endif // TUNINGSCHEMAVIEW_H
