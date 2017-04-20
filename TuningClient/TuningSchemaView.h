#ifndef TUNINGSCHEMAVIEW_H
#define TUNINGSCHEMAVIEW_H

#include "Stable.h"
#include "SchemaStorage.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/Tuning/TuningSignalManager.h"

class TuningSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit TuningSchemaView(SchemaStorage* schemaStorage, const QString &globalScript, QWidget* parent = nullptr);
	virtual ~TuningSchemaView();

protected:
	// Painting
	//
	virtual void paintEvent(QPaintEvent*) override;

	// Events
	//
	void timerEvent(QTimerEvent* event);

	// Public slots which are part of Script API
	//
public slots:
	virtual void setSchema(QString schemaId) override;

signals:
	void signal_setSchema(QString schemaId);


private:
	SchemaStorage* m_schemaStorage = nullptr;

};

#endif // TUNINGSCHEMAVIEW_H
