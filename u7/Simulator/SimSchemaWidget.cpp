#include "SimSchemaWidget.h"
#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "SimulatorTabPage.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemValue.h"
#include "../VFrame30/SchemaItemImageValue.h"
#include "../VFrame30/SchemaItemIndicator.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/MacrosExpander.h"

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


void SimSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	if (items.empty() == false)
	{
		for (const SchemaItemPtr& item : items)
		{
			if (VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
				schemaItemSignal != nullptr)
			{
				signalContextMenu(schemaItemSignal->appSignalIdList(), schemaItemSignal->impactAppSignalIdList(), {});
				break;
			}

			if (VFrame30::SchemaItemValue* schemaItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get());
				schemaItem != nullptr)
			{
				QStringList signalList = VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem);
				signalContextMenu(signalList, {}, {});
				break;
			}

			if (VFrame30::SchemaItemImageValue* schemaItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get());
				schemaItem != nullptr)
			{
				QStringList signalList = VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem);
				signalContextMenu(signalList, {}, {});
				break;
			}

			if (VFrame30::SchemaItemIndicator* schemaItem = dynamic_cast<VFrame30::SchemaItemIndicator*>(item.get());
				schemaItem != nullptr)
			{
				QStringList signalList = VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem);
				signalContextMenu(signalList, {}, {});
				break;
			}

			if (VFrame30::SchemaItemReceiver* schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
				schemaItemReceiver != nullptr)
			{
				signalContextMenu(schemaItemReceiver->appSignalIdsAsList(), {}, {});
				break;
			}

		}
	}

	return;
}

void SimSchemaWidget::signalContextMenu(const QStringList& appSignals, const QStringList& impactSignals, const QList<QMenu*> customMenu)
{
	// To set, it will sort list and exclude same ids
	//
	std::set<QString> signalListSet;
	for (const QString& s : appSignals)
	{
		signalListSet.insert(s);
	}

	std::set<QString> impactSignalListSet;
	for (const QString& s : impactSignals)
	{
		impactSignalListSet.insert(s);
	}

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

	// Compose menu
	//
	QMenu menu(this);

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : signalListSet)
	{
		QStringList schemaIds = m_simulator->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> impactSignalsSchemasSet;
	for (const QString& s : impactSignalListSet)
	{
		QStringList schemaIds = m_simulator->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}
	}

	if (signalsSchemasSet.empty() == true && impactSignalsSchemasSet.empty() == true)
	{
		schemasSubMenu->setDisabled(true);
	}
	else
	{
		for (const QString& schemaId : signalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, simWidget]() -> void
					 {
						simWidget->openSchemaTabPage(schemaId, appSignals + impactSignals);
					 };

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}

		if (signalsSchemasSet.empty() == false && impactSignalsSchemasSet.empty() == false)
		{
			schemasSubMenu->addSeparator();
		}

		for (const QString& schemaId : impactSignalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, simWidget]() -> void
					 {
						simWidget->openSchemaTabPage(schemaId, appSignals + impactSignals);
					 };

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}
	}

	// Custom menus
	//
	if (customMenu.isEmpty() == false)
	{
		for (auto cm : customMenu)
		{
			menu.addActions(cm->actions());
		}
	}

	// SignalInfo list
	//
	QAction* appSignalSeparator = menu.addSeparator();
	appSignalSeparator->setText(tr("Signals"));

	for (const QString& s : signalListSet)
	{
		bool ok = false;
		AppSignalParam signal =	m_simulator->appSignalManager().signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		QAction* a = menu.addAction(signalId);

		auto f = [this, s, simWidget]() -> void
				 {
					simWidget->signalInfo(s);
				 };

		connect(a, &QAction::triggered, this, f);
	}

	if (impactSignalListSet.empty() == false)
	{
		if (signalListSet.empty() == false)
		{
			QAction* impactSignalSeparator = menu.addSeparator();
			impactSignalSeparator->setText(tr("Impact Signals"));
		}

		for (const QString& s : impactSignalListSet)
		{
			bool ok = false;
			AppSignalParam signal =	m_simulator->appSignalManager().signalParam(s, &ok);

			QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

			QAction* a = menu.addAction(signalId);

			auto f = [this, s, simWidget]() -> void
					 {
						simWidget->signalInfo(s);
					 };

			connect(a, &QAction::triggered, this, f);
		}
	}

	// --
	//
	menu.exec(QCursor::pos());

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
