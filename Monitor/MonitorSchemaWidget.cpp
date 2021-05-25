#include "MonitorMainWindow.h"
#include "MonitorSchemaWidget.h"
#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "MonitorSignalInfo.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemValue.h"
#include "../VFrame30/SchemaItemImageValue.h"
#include "../VFrame30/SchemaItemIndicator.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemUfb.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/MacrosExpander.h"

//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
										 MonitorSchemaManager* schemaManager,
										 VFrame30::AppSignalController* appSignalController,
										 VFrame30::TuningController* tuningController,
                                         VFrame30::LogController* logController,
										 QWidget* parent) :
    VFrame30::ClientSchemaWidget(new MonitorView{schemaManager, appSignalController, tuningController, logController},
								 schema,
								 schemaManager,
								 parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenuRequested);

	createActions();

	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaWidget::createActions()
{
}

void MonitorSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	QStringList signalList;
	QStringList impactSignalList;

	for (const SchemaItemPtr& item : items)
	{
		if (VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
			schemaItemSignal != nullptr)
		{
			signalList.append(schemaItemSignal->appSignalIdList());
			impactSignalList.append(schemaItemSignal->impactAppSignalIdList());
			continue;
		}

		if (VFrame30::SchemaItemValue* schemaItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get());
			schemaItem != nullptr)
		{
			signalList.append(VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem));
			continue;
		}

		if (VFrame30::SchemaItemImageValue* schemaItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get());
			schemaItem != nullptr)
		{
			signalList.append(VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem));
			continue;
		}

		if (VFrame30::SchemaItemIndicator* schemaItem = dynamic_cast<VFrame30::SchemaItemIndicator*>(item.get());
			schemaItem != nullptr)
		{
			signalList.append(VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem));
			continue;
		}

		if (VFrame30::SchemaItemReceiver* schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
			schemaItemReceiver != nullptr)
		{
			signalList.append(schemaItemReceiver->appSignalIdsAsList());
			continue;
		}

		if (VFrame30::SchemaItemUfb* schemaItemUfb = dynamic_cast<VFrame30::SchemaItemUfb*>(item.get());
			schemaItemUfb != nullptr)
		{
			std::vector<std::shared_ptr<Property>> props = static_cast<const PropertyObject*>(schemaItemUfb)->specificProperties();

			for (auto p : props)
			{
				QString v = p->value().toString();
				if (v.startsWith(QChar('#')) == true)
				{
					signalList += v.split(QChar::LineFeed, Qt::SkipEmptyParts);
				}
			}

			continue;
		}
	}

	if (signalList.isEmpty() == false || impactSignalList.isEmpty() == false)
	{
		auto f = [](QString& s)
			{
				if (s.startsWith('@') == true)
				{
					s = theSignals.equipmentToAppSiganlId(s);
				}
			};

		std::ranges::for_each(signalList, f);
		std::ranges::for_each(impactSignalList, f);

		signalContextMenu(signalList, impactSignalList, {});
	}

	return;
}

void MonitorSchemaWidget::signalContextMenu(const QStringList& appSignals, const QStringList& impactSignals, const QList<QMenu*>& customMenu)
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

	// Compose menu
	//
	QMenu menu(this);

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : signalListSet)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> impactSignalsSchemasSet;
	for (const QString& s : impactSignalListSet)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByAppSignalId(s);

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
			auto f = [this, schemaId, &appSignals, &impactSignals]() -> void
					 {
						setSchema(schemaId, appSignals + impactSignals);
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
			auto f = [this, schemaId, &appSignals, &impactSignals]() -> void
					 {
						if (schemaId != this->schemaId())
						{
							setSchema(schemaId, appSignals + impactSignals);
						}
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
		AppSignalParam signal =	theSignals.signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		QAction* a = menu.addAction(signalId);

		auto f = [this, &signal]() -> void
				 {
					signalInfo(signal.appSignalId());
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

			AppSignalParam signal =	theSignals.signalParam(s, &ok);

			QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

			QAction* a = menu.addAction(signalId);

			auto f = [this, &signal]() -> void
					 {
						signalInfo(signal.appSignalId());
					 };

			connect(a, &QAction::triggered, this, f);
		}
	}

	// --
	//

	menu.exec(QCursor::pos());

	return;
}

void MonitorSchemaWidget::signalInfo(QString appSignalId)
{
	MonitorSignalInfo::showDialog(appSignalId,
	                             theMonitorMainWindow->configController(),
	                             theMonitorMainWindow->tcpSignalClient(),
	                             theMonitorMainWindow->monitorCentralWidget());

	return;
}

MonitorView* MonitorSchemaWidget::monitorSchemaView()
{
	MonitorView* result = dynamic_cast<MonitorView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

const MonitorView* MonitorSchemaWidget::monitorSchemaView() const
{
	const MonitorView* result = dynamic_cast<const MonitorView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

MonitorSchemaManager* MonitorSchemaWidget::schemaManager()
{
	return monitorSchemaView()->monitorSchemaManager();
}

const MonitorSchemaManager* MonitorSchemaWidget::schemaManager() const
{
	return monitorSchemaView()->monitorSchemaManager();
}
