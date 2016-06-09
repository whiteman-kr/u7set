#include <QMessageBox>
#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"
#include "SchemaManager.h"
#include "DialogSignalInfo.h"
#include "../VFrame30/SchemaItemSignal.h"

//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaManager* schemaManager) :
	BaseSchemaWidget(schema, new MonitorSchemaView(schemaManager)),
	m_schemaManager(schemaManager)
{
	assert(m_schemaManager);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenu);

	createActions();

	connect(schemaView(), &VFrame30::SchemaView::signal_schemaChanged, this, [this](VFrame30::Schema* schema)
		{
			emit this->signal_schemaChanged(this, schema);
		});

	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
}

void MonitorSchemaWidget::createActions()
{
	// New tab (duplicate this one)
	//
//	m_newTabAction = new QAction(tr("New Tab"), this);
//	m_newTabAction->setEnabled(true);
//	QList<QKeySequence> newTabShortcuts;
//	newTabShortcuts << QKeySequence::AddTab;
//	newTabShortcuts << QKeySequence::New;
//	m_newTabAction->setShortcuts(newTabShortcuts);
//	//m_newTabAction->setShortcutContext(Qt::WidgetShortcut);		// To avoid abigious with main menu same QAction
//	addAction(m_newTabAction);

//	connect(m_newTabAction, &QAction::triggered, this, [this](){ emit signal_newTab(this);});

//	// Closet current tab
//	//
//	m_closeTabAction = new QAction(tr("Close Tab"), this);
//	m_closeTabAction->setEnabled(true);
//	//m_closeTabAction->setShortcuts(QKeySequence::Close);			// To avoid abigious with main menu same QAction
//	addAction(m_closeTabAction);

//	connect(m_closeTabAction, &QAction::triggered, this, [this](){ emit signal_closeTab(this);});

	// --
	//

}

std::vector<std::shared_ptr<VFrame30::SchemaItem>> MonitorSchemaWidget::itemsUnderCursor(const QPoint& pos)
{
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> result;
	result.reserve(8);

	QPointF docPoint;

	bool convertResult = MousePosToDocPoint(pos, &docPoint);
	if (convertResult == false)
	{
		return result;
	}

	double x = docPoint.x();
	double y = docPoint.y();

	for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
		{
			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

			if (item->IsIntersectPoint(x, y) == true)
			{
				result.push_back(item);
			}
		}
	}

	return result;
}

QString MonitorSchemaWidget::schemaId() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->schemaID();
}

QString MonitorSchemaWidget::caption() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->caption();
}

void MonitorSchemaWidget::contextMenu(const QPoint& pos)
{
	// Disable/enable actions
	//

	//m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);

	// Compose menu
	//
	QMenu menu(this);
	QList<QAction*> actions;

	// Signals items
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items = itemsUnderCursor(pos);

	if (items.empty() == false)
	{
		for (const std::shared_ptr<VFrame30::SchemaItem>& item : items)
		{
			VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());

			if (schemaItemSignal != nullptr)
			{
				const QStringList& signalList = schemaItemSignal->appSignalIdList();

				for (const QString& s: signalList)
				{
					Signal signal;
					bool ok = theSignals.signal(s, &signal);

					QString signalId = ok ? QString("%1 %2").arg(signal.customAppSignalID()).arg(signal.caption()) : s;

					QAction* a = new QAction(signalId, &menu);

					auto f = [this, s]() -> void
							 {
								signalInfo(s);
							 };

					connect(a, &QAction::triggered, this, f);

					actions << a;
				}
			}
		}
	}

//
//	actions << m_newTabAction;
//	actions << m_closeTabAction;

	menu.exec(actions, mapToGlobal(pos), 0, this);

	return;
}

void MonitorSchemaWidget::signalInfo(QString appSignalId)
{
	qDebug() << "MonitorSchemaWidget::signalInfo:  " << appSignalId;

	Signal signal;
	bool ok = theSignals.signal(appSignalId, &signal);
	if (ok == true)
	{
		DialogSignalInfo* dsi = new DialogSignalInfo(this, signal);
		dsi->show();
	}
	else
	{
		QMessageBox::critical(this, tr("Error"), tr("No information about this signal!"));
	}

	return;
}



