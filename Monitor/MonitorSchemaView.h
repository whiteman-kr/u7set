#pragma once

#include <QJSEngine>
#include "../VFrame30/SchemaView.h"

class MonitorSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit MonitorSchemaView(QWidget* parent = nullptr);
	virtual ~MonitorSchemaView();

	// Methods
	//
public:

protected:
	void runScript(const QString& script, VFrame30::SchemaItem* schemaItem);

	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

	// Events
	//
protected:
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

signals:

public slots:

	// Data
	//
private:
	std::shared_ptr<VFrame30::SchemaItem> m_leftClickOverItem;

	QJSEngine m_jsEngine;
};

