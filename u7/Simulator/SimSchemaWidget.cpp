#include "SimSchemaWidget.h"
#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "SimulatorTabPage.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/AppSignalController.h"

//
//
//	SimSchemaWidget
//
//

SimSchemaWidget::SimSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
								 SimSchemaManager* schemaManager,
								 VFrame30::AppSignalController* appSignalController,
								 VFrame30::TuningController* tuningController,
                                 SimIdeSimulator* simulator,
                                 QWidget* parent) :
    VFrame30::ClientSchemaWidget(new SimSchemaView{schemaManager}, schema, schemaManager, parent),
	m_simulator(simulator)
{
	Q_ASSERT(m_simulator);

	clientSchemaView()->setAppSignalController(appSignalController);
	clientSchemaView()->setTuningController(tuningController);

	setFrameShape(QFrame::NoFrame);

	// --
	//
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &SimSchemaWidget::contextMenuRequested);

	createActions();

	// --
	//
	connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimSchemaWidget::updateSchema);

	return;
}

SimSchemaWidget::~SimSchemaWidget()
{
}

void SimSchemaWidget::createActions()
{
	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setStatusTip(tr("Zoom in schema view"));
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, this, &SimSchemaWidget::zoomIn);

	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setStatusTip(tr("Zoom out schema view"));
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, this, &SimSchemaWidget::zoomOut);

	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setStatusTip(tr("Set zoom to 100%"));
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, this, &SimSchemaWidget::zoom100);

	m_zoomToFitAction = new QAction(tr("Fit to Screen"), this);
	m_zoomToFitAction->setStatusTip(tr("Set zoom to fit the screen"));
	m_zoomToFitAction->setEnabled(true);
	m_zoomToFitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Slash));
	connect(m_zoomToFitAction, &QAction::triggered, this, &SimSchemaWidget::zoomToFit);

	addAction(m_zoomInAction);
	addAction(m_zoomOutAction);
	addAction(m_zoom100Action);
	addAction(m_zoomToFitAction);

	return;
}

void SimSchemaWidget::signalContextMenu(const QStringList signalList)
{
	SimWidget* simWidget = nullptr;
	QWidget* parent = this->parentWidget();
	while (parent != nullptr)
	{
		simWidget = dynamic_cast<SimWidget*>(parent);
		if (simWidget != nullptr)
		{
			break;
		}

		parent = parent->parentWidget();
	}

	if (simWidget == nullptr)
	{
		Q_ASSERT(simWidget);
		return;
	}

	simWidget->signalContextMenu(signalList, {});

	return;
}

void SimSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Disable/enable actions
	//

	//m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);


	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	if (items.empty() == false)
	{
		for (const SchemaItemPtr& item : items)
		{
			VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());

			if (schemaItemSignal != nullptr)
			{
				const QStringList& signalList = schemaItemSignal->appSignalIdList();

				signalContextMenu(signalList);
				break;
			}

			VFrame30::SchemaItemReceiver* schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());

			if (schemaItemReceiver != nullptr)
			{
				signalContextMenu(schemaItemReceiver->appSignalIdsAsList());
				break;
			}

		}
	}

//
//	actions << m_newTabAction;
//	actions << m_closeTabAction;

	return;
}

void SimSchemaWidget::updateSchema()
{
	if (m_simulator->isLoaded() == true)
	{
		auto newSchema = schemaManager()->schema(schemaId());
		BaseSchemaWidget::setSchema(newSchema, true);
	}

	return;
}

SimSchemaView* SimSchemaWidget::simSchemaView()
{
	SimSchemaView* result = dynamic_cast<SimSchemaView*>(schemaView());
	assert(result);
	return result;
}

const SimSchemaView* SimSchemaWidget::simSchemaView() const
{
	const SimSchemaView* result = dynamic_cast<const SimSchemaView*>(schemaView());
	assert(result);
	return result;
}
