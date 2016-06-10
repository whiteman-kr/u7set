#pragma once

#include "../VFrame30/BaseSchemaWidget.h"

class MonitorSchemaView;
class SchemaManager;

namespace VFrame30
{
	class SchemaItem;
}

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

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsUnderCursor(const QPoint& pos);

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
	void contextMenuRequested(const QPoint &pos);

public slots:
	void signalInfo(QString appSignalId);

	// Properties
	//
public:
	//MonitorSchemaView* schemaView();
	//const MonitorSchemaView* schemaView() const;
	void signalContextMenu(const QStringList signalList);

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

