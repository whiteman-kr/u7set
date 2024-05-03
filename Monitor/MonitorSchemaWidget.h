#pragma once

#include "../VFrame30/ClientSchemaWidget.h"

class MonitorSchemaView;
class MonitorSchemaManager;
struct SchemaHistoryItem;

namespace ClientLib
{
	class AppSignalManager;
}

namespace VFrame30
{
	class AppSignalController;
	class ITimeStats;
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
						VFrame30::LogController* logController,
						VFrame30::ITimeStats* timeStats,
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
	IAppSignalManager& appSignalManager();
	const IAppSignalManager& appSignalManager() const;

	ClientLib::AppSignalManager& clientAppSignalManager();
	const ClientLib::AppSignalManager& clientAppSignalManager() const;

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
	VFrame30::ITimeStats* m_timeStats = nullptr;

};

