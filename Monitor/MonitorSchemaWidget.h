#pragma once

#include "MonitorSchemaManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"

class MonitorView;
struct SchemaHistoryItem;

namespace VFrame30
{
	class SchemaItem;
}

//
//
// MonitorSchemaWidget
//
//
class MonitorSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

private:
	MonitorSchemaWidget() = delete;

public:
	MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
						MonitorSchemaManager* schemaManager,
						VFrame30::AppSignalController* appSignalController,
						VFrame30::TuningController* tuningController);
	virtual ~MonitorSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:

	// --
	//
protected:


	// Signals
	//
signals:
	//void signal_newTab(MonitorSchemaWidget* tabWidget);			// Command to the owner to duplicate current tab
	//void signal_closeTab(MonitorSchemaWidget* tabWidget);		// Command to the owner to Close current tab

	//void signal_schemaChanged(MonitorSchemaWidget* tabWidget, VFrame30::Schema* schema);

	//void signal_historyChanged(bool enableBack, bool enableForward);

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint &pos);

	void signalContextMenu(const QStringList signalList);

	void signalInfo(QString appSignalId);

	// Properties
	//
public:
	MonitorView* monitorSchemaView();
	const MonitorView* monitorSchemaView() const;

	// Data
	//
private:

	// Actions
	//
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;
};

