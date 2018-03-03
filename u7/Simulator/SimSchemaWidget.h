#pragma once

#include "SimSchemaManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"

class SimSchemaView;

namespace VFrame30
{
	class SchemaItem;
}

//
//
// SimSchemaWidget
//
//
class SimSchemaWidget : public VFrame30::ClientSchemaWidget
{
	Q_OBJECT

private:
	SimSchemaWidget() = delete;

public:
	SimSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
					SimSchemaManager* schemaManager,
					VFrame30::AppSignalController* appSignalController,
					VFrame30::TuningController* tuningController);
	virtual ~SimSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:
protected:

	// Signals
	//
signals:

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint &pos);
	void signalContextMenu(const QStringList signalList);

	void signalInfo(QString appSignalId);

	// Properties
	//
public:
	SimSchemaView* simSchemaView();
	const SimSchemaView* simSchemaView() const;

	// Data
	//
private:

	// Actions
	//
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;
};

