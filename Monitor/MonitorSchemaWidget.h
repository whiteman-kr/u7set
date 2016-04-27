#pragma once

#include "../VFrame30/BaseSchemaWidget.h"

class MonitorSchemaView;
class SchemaManager;

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
	MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaManager* schemaManager);
	virtual ~MonitorSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:
	QString schemaId() const;
	QString caption() const;

	// Events
	//
protected:

	// Signals
	//
signals:
	//void signal_newTab(MonitorSchemaWidget* tabWidget);			// Command to the owner to duplicate current tab
	//void signal_closeTab(MonitorSchemaWidget* tabWidget);		// Command to the owner to Close current tab

	void signal_schemaChanged(MonitorSchemaWidget* tabWidget, VFrame30::Schema* schema);

	// Slots
	//
protected slots:
	void contextMenu(const QPoint &pos);

	// Properties
	//
public:
	//MonitorSchemaView* schemaView();
	//const MonitorSchemaView* schemaView() const;

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;

	// Actions
	//
private:
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;
};

