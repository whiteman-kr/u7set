#ifndef TUNINGSCHEMAVIEW_H
#define TUNINGSCHEMAVIEW_H

#include "SchemaStorage.h"
#include "../VFrame30/SchemaView.h"
#include "../lib/Tuning/TuningSignalManager.h"

class TuningSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit TuningSchemaView(SchemaStorage* schemaStorage, const QString& globalScript, QWidget* parent = nullptr);
	virtual ~TuningSchemaView();

protected:
	// Painting
	//
	virtual void paintEvent(QPaintEvent*) override;

	virtual void timerEvent(QTimerEvent*) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;


	// Public slots which are part of Script API
	//
public slots:
	virtual void setSchema(QString schemaId) override;

signals:
	void signal_setSchema(QString schemaId);

private slots:
	void startRepaintTimer();

private:
	SchemaStorage* m_schemaStorage = nullptr;

	std::shared_ptr<VFrame30::SchemaItem> m_leftClickOverItem;

	QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();
};

#endif // TUNINGSCHEMAVIEW_H
