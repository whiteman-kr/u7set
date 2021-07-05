#include "MonitorMainWindow.h"
#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"
#include "MonitorSchemaManager.h"
#include "MonitorSignalInfo.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemValue.h"
#include "../VFrame30/SchemaItemImageValue.h"
#include "../VFrame30/SchemaItemIndicator.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemUfb.h"
#include "../VFrame30/SchemaItemLoopback.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/MacrosExpander.h"

//
//MonitorTuningController
//

MonitorTuningController::MonitorTuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, TuningUserManager* tuningUserManager, QWidget* parent):
	VFrame30::TuningController(signalManager, tcpClient, parent),
	m_tuningUserManager(tuningUserManager),
	m_parentWidget(parent)
{

}


bool MonitorTuningController::checkTuningAccess() const
{
	if (m_tuningUserManager == nullptr)
	{
		Q_ASSERT(m_tuningUserManager);
		return false;
	}

	if (m_tuningUserManager->login(m_parentWidget) == false)
	{
		return false;
	}

	return true;
}

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
    VFrame30::ClientSchemaWidget(new MonitorSchemaView{schemaManager, appSignalController, tuningController, logController},
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
	QStringList loopbacks;

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

		if (VFrame30::SchemaItemLoopback* schemaItemLoopback = dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get());
			schemaItemLoopback != nullptr)
		{
			loopbacks.push_back(schemaItemLoopback->loopbackId());
			continue;
		}
	}

	if (signalList.isEmpty() == false || impactSignalList.isEmpty() == false || loopbacks.isEmpty() == false)
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

		signalContextMenu(signalList, impactSignalList, loopbacks, {});
	}

	return;
}

void MonitorSchemaWidget::signalContextMenu(QStringList appSignals,
											QStringList impactSignals,
											QStringList loopbacks,
											const QList<QMenu*>& customMenu)
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

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : appSignals)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> impactSignalsSchemasSet;
	for (const QString& s : impactSignals)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> loopbackSchemas;
	for (const QString& l : loopbacks)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByLoopbackId(l);

		for (const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}
	}

	// --
	//
	if (signalsSchemasSet.empty() == true &&
		impactSignalsSchemasSet.empty() == true &&
		loopbackSchemas.empty() == true)
	{
		schemasSubMenu->setDisabled(true);
	}
	else
	{
		for (const QString& schemaId : signalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
					{
						if (schemaId != this->schemaId())
						{
							setSchema(schemaId, appSignals + impactSignals + loopbacks);
						}
					};

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}

		schemasSubMenu->addSeparator();

		for (const QString& schemaId : impactSignalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
					{
						if (schemaId != this->schemaId())
						{
							setSchema(schemaId, appSignals + impactSignals + loopbacks);
						}
					};

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}

		// Loopbacks
		//
		schemasSubMenu->addSeparator();

		for (const QString& schemaId : loopbackSchemas)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
					{
						if (schemaId != this->schemaId())
						{
							setSchema(schemaId, appSignals + impactSignals + loopbacks);
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

	for (const QString& s : appSignals)
	{
		bool ok = false;
		AppSignalParam signal =	theSignals.signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		QAction* a = menu.addAction(signalId);

		auto f = [this, signal]() -> void
				 {
					signalInfo(signal.appSignalId());
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

MonitorSchemaView* MonitorSchemaWidget::monitorSchemaView()
{
	MonitorSchemaView* result = dynamic_cast<MonitorSchemaView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

const MonitorSchemaView* MonitorSchemaWidget::monitorSchemaView() const
{
	const MonitorSchemaView* result = dynamic_cast<const MonitorSchemaView*>(schemaView());
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
