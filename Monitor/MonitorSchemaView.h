#pragma once

#include <QJSEngine>
#include "../VFrame30/SchemaView.h"

class SchemaManager;

namespace VFrame30
{
	class SchemaItem;
}

class MonitorSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit MonitorSchemaView(SchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~MonitorSchemaView();

	// Methods
	//
public:

protected:
	Q_INVOKABLE bool setSchema(QString schemaId);

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

public slots:

	// Properties
	//
public:
	virtual QString globalScript() const override;

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;

	std::shared_ptr<VFrame30::SchemaItem> m_leftClickOverItem;
};



