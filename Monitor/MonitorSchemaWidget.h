#pragma once

#include "MonitorSchemaManager.h"
#include "MonitorSignalManager.h"
#include "../ClientLib/TuningConnection.h"
#include "../ClientLib/TuningUserManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"
#include "../lib/ITimeStats.h"


class MonitorSchemaView;
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
						VFrame30::LogController* logController,
						ITimeStats* timeStats,
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
	void signalContextMenu(QStringList appSignals,
						   QStringList impactSignals,
						   QStringList loopbacks,
						   const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

	// Properties
	//
public:
	IAppSignalManager* signalManager();
	const IAppSignalManager* signalManager() const;

	MonitorSignalManager* monitorSignalManager();
	const MonitorSignalManager* monitorSignalManager() const;

	MonitorSchemaView* monitorSchemaView();
	const MonitorSchemaView* monitorSchemaView() const;

	MonitorSchemaManager* schemaManager();
	const MonitorSchemaManager* schemaManager() const;

	// Data
	//
private:

	// Actions
	//
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

	// Data access
	VFrame30::LogController* m_logController = nullptr;
	ITimeStats* m_timeStats = nullptr;

};

