#pragma once

#include "DiagnosticsSchemaManager.h"
//#include "MonitorSignalManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
// -- #include "../VFrame30/AppSignalController.h"
#include "../lib/ITimeStats.h"


class DiagSchemaView;
struct SchemaHistoryItem;

//
//
// DiagSchemaWidget
//
//
class DiagSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

public:
	DiagSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
					 DiagnosticsSchemaManager* schemaManager,
						// -- VFrame30::AppSignalController* appSignalController,
						VFrame30::LogController* logController,
						ITimeStats* timeStats,
						QWidget* parent);
	virtual ~DiagSchemaWidget();

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
	// -- IAppSignalManager* signalManager();
	// -- const IAppSignalManager* signalManager() const;

	// -- MonitorSignalManager* monitorSignalManager();
	// -- const MonitorSignalManager* monitorSignalManager() const;

	DiagSchemaView* diagSchemaView();
	const DiagSchemaView* diagSchemaView() const;

	DiagnosticsSchemaManager* schemaManager();
	const DiagnosticsSchemaManager* schemaManager() const;

	// Data
	//
private:

	// Actions
	//
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

	// Data access
	//
	VFrame30::LogController* m_logController = nullptr;
	ITimeStats* m_timeStats = nullptr;
};

