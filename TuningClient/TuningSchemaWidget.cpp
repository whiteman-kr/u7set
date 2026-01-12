#include "TuningSchemaWidget.h"
#include "Main.h"
#include "MainWindow.h"
#include "TuningSchemaView.h"
#include "TuningSignalInfo.h"

#include <VFrame30/Context.h>
#include <VFrame30/SchemaItemConnection.h>
#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemIndicator.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemSignal.h>
#include <VFrame30/SchemaItemUfb.h>
#include <VFrame30/SchemaItemValue.h>

//
// TuningSchemaWidget
//
TuningSchemaWidget::TuningSchemaWidget(TuningConfigController& configController,
									   VFrame30::LogController* logController,
									   std::shared_ptr<VFrame30::Schema> schema,
									   TuningSchemaManager& schemaManager,
									   QWidget* parent) :
	VFrame30::ClientSchemaWidget(new TuningSchemaView{configController, schemaManager}, schema, &schemaManager, parent),
	m_configController(configController)
{
	clientSchemaView()->setTuningController(theApp.mainWindow()->tuningSignalManager(),
											theApp.mainWindow()->tuningConnection(),
											theApp.mainWindow()->tuningAuthorization());
	clientSchemaView()->setLogController(logController);
	clientSchemaView()->setZoom(100, false);

	// Run onShowScript
	//
	Q_ASSERT(schema);
	schema->setContext(VFrame30::Context::create(clientSchemaView()));

	tuningSchemaView()->configurationArrived(m_configController.configuration());

	schema->onShowEvent(clientSchemaView()->jsEngine(), clientSchemaView()->logFile());

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &TuningSchemaWidget::contextMenuRequested);

	return;
}


void TuningSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Reset highlights
	//
	clientSchemaView()->setHighlightSignalIds({});

	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	QStringList signalList;
	QStringList impactSignalList;
	QStringList loopbacks;

	auto context = VFrame30::Context::create(clientSchemaView());

	for (const SchemaItemPtr& item : items)
	{
		if (auto schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()); schemaItemSignal != nullptr)
		{
			signalList.append(schemaItemSignal->appSignalIdList());
			impactSignalList.append(schemaItemSignal->impactAppSignalIdList());
			continue;
		}

		if (auto schemaItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get()); schemaItem != nullptr)
		{
			signalList.append(schemaItem->signalIds(context.get()));
			continue;
		}

		if (auto schemaItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get()); schemaItem != nullptr)
		{
			signalList.append(schemaItem->signalIds(context.get()));
			continue;
		}

		if (auto schemaItem = dynamic_cast<VFrame30::SchemaItemIndicator*>(item.get()); schemaItem != nullptr)
		{
			signalList.append(schemaItem->signalIds(context.get()));
			continue;
		}

		if (auto schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()); schemaItemReceiver != nullptr)
		{
			signalList.append(schemaItemReceiver->appSignalIdsAsList());
			continue;
		}

		if (auto schemaItemUfb = dynamic_cast<VFrame30::SchemaItemUfb*>(item.get()); schemaItemUfb != nullptr)
		{
			std::vector<std::shared_ptr<Property>> props = static_cast<const PropertyObject*>(schemaItemUfb)->specificProperties();

			for (const auto& p : props)
			{
				QString v = p->value().toString();
				if (v.startsWith(QChar('#')) == true)
				{
					signalList += v.split(QChar::LineFeed, Qt::SkipEmptyParts);
				}
			}

			continue;
		}

		if (VFrame30::SchemaItemLoopback* schemaItemLoopback = dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get());
			schemaItemLoopback != nullptr)
		{
			loopbacks.push_back(schemaItemLoopback->loopbackId());
			continue;
		}
	}

	signalContextMenu(signalList, impactSignalList, loopbacks, {});

	return;
}

void TuningSchemaWidget::signalContextMenu(QStringList appSignals,
										   QStringList impactSignals,
										   QStringList loopbacks,
										   const QList<QMenu*>& /*customMenu*/)
{
	appSignals.sort();
	appSignals.removeDuplicates();

	impactSignals.sort();
	impactSignals.removeDuplicates();

	loopbacks.sort();
	loopbacks.removeDuplicates();

	// Compose menu
	//
	QMenu menu(this);

	// Reset highlights.
	//
	if (clientSchemaView()->hasHighlightItems() == true)
	{
		QAction* a = menu.addAction(tr("Reset Highlights"));
		a->setEnabled(clientSchemaView()->hasHighlightItems());
		connect(a,
				&QAction::triggered,
				this,
				[this]()
				{
					clientSchemaView()->clearHighlightItems();
				});
	}

	if (appSignals.isEmpty() == true && impactSignals.isEmpty() == true && loopbacks.isEmpty() == true &&
		clientSchemaView()->hasHighlightItems() == false)
	{
		return;
	}

	// SignalInfo list
	//
	QAction* appSignalSeparator = menu.addSeparator();
	appSignalSeparator->setText(tr("Signals"));

	for (const QString& s : appSignals)
	{
		auto signal = theApp.mainWindow()->tuningSignalManager().signalParam(s);

		QString signalId = signal.has_value() ? QString("%1 %2").arg(signal->customSignalId()).arg(signal->caption()) : s;

		QAction* a = menu.addAction(signalId);

		auto f = [this, s]() -> void
		{
			signalInfo(s);
		};

		connect(a, &QAction::triggered, this, f);
	}

	if (impactSignals.empty() == false)
	{
		if (appSignals.empty() == false)
		{
			QAction* impactSignalSeparator = menu.addSeparator();
			impactSignalSeparator->setText(tr("Impact Signals"));
		}

		for (const QString& s : impactSignals)
		{
			auto signal = theApp.mainWindow()->tuningSignalManager().signalParam(s);

			QString signalId = signal.has_value() ? QString("%1 %2").arg(signal->customSignalId()).arg(signal->caption()) : s;

			QAction* a = menu.addAction(signalId);

			auto f = [this, s]() -> void
			{
				signalInfo(s);
			};

			connect(a, &QAction::triggered, this, f);
		}
	}

	// --
	//

	menu.exec(QCursor::pos());

	return;
}

void TuningSchemaWidget::signalInfo(QString appSignalId)
{
	TuningSignalInfo* d = new TuningSignalInfo(m_configController,
											   theApp.mainWindow()->tuningSignalManager(),
											   theApp.mainWindow()->tuningConnection(),
											   ::calcHash(appSignalId),
											   E::AnalogFormat::g_9_or_9e,
											   this);
	d->show();

	return;
}

TuningSchemaView* TuningSchemaWidget::tuningSchemaView()
{
	return static_cast<TuningSchemaView*>(clientSchemaView());
}

const TuningSchemaView* TuningSchemaWidget::tuningSchemaView() const
{
	return static_cast<const TuningSchemaView*>(clientSchemaView());
}
