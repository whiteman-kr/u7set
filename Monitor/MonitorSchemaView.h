#pragma once

#include "../VFrame30/SchemaView.h"


class MonitorSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit MonitorSchemaView(QWidget* parent = nullptr);
	virtual ~MonitorSchemaView();

	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

signals:

public slots:
};

