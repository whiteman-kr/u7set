#pragma once

#include "../VFrame30/BaseSchemaWidget.h"

class MonitorSchemaView;

//
//
// MonitorSchemaWidget
//
//
class MonitorSchemaWidget : public VFrame30::BaseSchemaWidget
{
	Q_OBJECT

private:
	MonitorSchemaWidget() = delete;

public:
	MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema);
	virtual ~MonitorSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:
	QString schemaId() const;
	QString caption() const;

	// Signals
	//
signals:
	//	void closeTab(QWidget* tabWidget);		// Command to the owner to Close current tab

	// Slots
	//
protected slots:

	// Properties
	//
public:
	//MonitorSchemaView* schemaView();
	//const MonitorSchemaView* schemaView() const;


	// Data
	//
private:

	// Actions
	//
private:
};

