#pragma once

#include "../VFrame30/SchemeView.h"


class MonitorSchemaView : public VFrame30::SchemeView
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

