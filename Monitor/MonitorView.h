#pragma once

#include <QJSEngine>
#include "../VFrame30/SchemaView.h"

class SchemaManager;

namespace VFrame30
{
	class SchemaItem;
}

class MonitorView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit MonitorView(SchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~MonitorView();

	// Methods
	//
public:

	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

	void timerEvent(QTimerEvent* event);

	// Events
	//
protected:
	//virtual void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

signals:
	void signal_setSchema(QString schemaId);

	// Public slots which are part of Script API
	//
public slots:
	virtual void setSchema(QString schemaId) override;

protected slots:
	void startRepaintTimer();

	// Properties
	//
public:
	virtual QString globalScript() const override;
	virtual QJSEngine* jsEngine() override;

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;

	std::shared_ptr<VFrame30::SchemaItem> m_leftClickOverItem;

	QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();
};


