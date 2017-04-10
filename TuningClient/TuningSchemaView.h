#ifndef TUNINGSCHEMAVIEW_H
#define TUNINGSCHEMAVIEW_H

#include "Stable.h"
#include "SchemaStorage.h"
#include "../VFrame30/SchemaView.h"

class TuningSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit TuningSchemaView(SchemaStorage* schemaStorage, QWidget* parent = nullptr);
	virtual ~TuningSchemaView();

protected:
	// Painting
	//
	virtual void paintEvent(QPaintEvent*) override;

	// Events
	//
	void timerEvent(QTimerEvent* event);

public:

	void setSchema(QString schemaId);

signals:
	void signal_setSchema(QString schemaId);


private:
	SchemaStorage* m_schemaStorage = nullptr;


};

#endif // TUNINGSCHEMAVIEW_H
