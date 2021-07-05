#pragma once

#include "MonitorSchemaManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"
#include "../lib/Tuning/TuningUserManager.h"


class MonitorSchemaView;
struct SchemaHistoryItem;

class MonitorTuningController : public VFrame30::TuningController
{
	Q_OBJECT

public:
	MonitorTuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, TuningUserManager* tuningUserManager, QWidget* parent = nullptr);

protected:
	virtual bool checkTuningAccess() const override;

private:
	QWidget* m_parentWidget = nullptr;
	TuningUserManager* m_tuningUserManager = nullptr;
};

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
	void signalContextMenu(QStringList appSignals,
						   QStringList impactSignals,
						   QStringList loopbacks,
						   const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

	// Properties
	//
public:
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
};

