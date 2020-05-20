#pragma once

#include "SimSchemaManager.h"
#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"

class SimSchemaView;
class SimIdeSimulator;

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
					VFrame30::TuningController* tuningController,
					SimIdeSimulator* simulator);
	virtual ~SimSchemaWidget();

protected:
	void createActions();

	// Methods
	//
public:
	void signalContextMenu(const QStringList signalList);

protected:

	// Signals
	//
signals:

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint &pos);

protected slots:
	void updateSchema();

	// Properties
	//
public:
	SimSchemaView* simSchemaView();
	const SimSchemaView* simSchemaView() const;

	// Data
	//
private:
	SimIdeSimulator* m_simulator = nullptr;

	// Actions
	//
	QAction* m_zoomInAction = nullptr;
	QAction* m_zoomOutAction = nullptr;
	QAction* m_zoom100Action = nullptr;
	QAction* m_zoomToFitAction = nullptr;

	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;
};

