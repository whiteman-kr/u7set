#ifndef TUNINGSCHEMAWIDGET_H
#define TUNINGSCHEMAWIDGET_H

#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/SchemaView.h"
#include "TuningSchemaView.h"
#include "TuningSchemaManager.h"


class TuningClientTuningController : public VFrame30::TuningController
{
	Q_OBJECT

public:
	TuningClientTuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QObject* parent = nullptr);

protected:
	virtual bool writingEnabled() const override;
};


class TuningSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

private:
	TuningSchemaWidget() = delete;

public:
	TuningSchemaWidget(TuningSignalManager* tuningSignalManager,
					   TuningClientTuningController* tuningController,
					   std::shared_ptr<VFrame30::Schema> schema,
					   TuningSchemaManager* schemaManager,
					   QWidget* parent);
	virtual ~TuningSchemaWidget();

	//TuningSchemaView* tuningSchemaView();

private:
};

#endif // TUNINGSCHEMAWIDGET_H
