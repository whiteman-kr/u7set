#pragma once

#include "MonitorSchemaManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"


class MonitorView;
struct SchemaHistoryItem;


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
						VFrame30::TuningController* tuningController,
						VFrame30::LogController* logController,
						QWidget* parent);
	virtual ~MonitorSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint& pos);
	void signalContextMenu(const QStringList& appSignals, const QStringList& impactSignals, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

	// Properties
	//
public:
	MonitorView* monitorSchemaView();
	const MonitorView* monitorSchemaView() const;

	MonitorSchemaManager* schemaManager();
	const MonitorSchemaManager* schemaManager() const;

	// Data
	//
private:

	// Actions
	//
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;
};

