#include "SimProjectWidget.h"
#include "../Settings.h"

using namespace SimProjectTreeItems;

SimProjectWidget::SimProjectWidget(SimIdeSimulator* simulator, QWidget* parent) :
	QWidget(parent),
	m_simulator(simulator)
{
	assert(simulator);

	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);

	QVBoxLayout* layout = new QVBoxLayout;

	m_buildLabel = new QLabel("Build: Not loaded");
	m_buildLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
	m_buildLabel->setTextFormat(Qt::RichText);
	m_buildLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_buildLabel->setOpenExternalLinks(true);

	m_treeWidget = new QTreeWidget;
	m_treeWidget->setUniformRowHeights(true);
	m_treeWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	QStringList headerLabels;
	headerLabels << "ID";
	headerLabels << "Info";
	headerLabels << "State";
	m_treeWidget->setHeaderLabels(headerLabels);

	QByteArray headerState = QSettings().value("SimulatorProjectWidget/headerState").toByteArray();
	m_treeWidget->header()->restoreState(headerState);

	layout->addWidget(m_buildLabel);
	layout->addWidget(m_treeWidget);

	setLayout(layout);

	// --
	//
	createActions();

	projectUpdated();	// Fill controls on start new window

	// --
	//
	connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &SimProjectWidget::treeContextMenu);
	connect(m_treeWidget, &QTreeWidget::doubleClicked, this, &SimProjectWidget::treeDoubleClicked);

	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimProjectWidget::projectUpdated);

	connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimProjectWidget::updateModuleStates);

	return;
}

SimProjectWidget::~SimProjectWidget()
{
	QByteArray headerState = m_treeWidget->header()->saveState();
	QSettings().setValue("SimulatorProjectWidget/headerState", headerState);

	return;
}

void SimProjectWidget::createActions()
{
	m_openLmControlPageAction = new QAction(tr("Control Page..."), this);
	connect(m_openLmControlPageAction, &QAction::triggered, this, &SimProjectWidget::openModuleTabPage);

	m_openLmCodePageAction = new QAction(tr("App Code..."), this);
	connect(m_openLmCodePageAction, &QAction::triggered, this, &SimProjectWidget::openCodeTabPage);

	return;
}

void SimProjectWidget::projectUpdated()
{
	assert(m_buildLabel);

	if (m_simulator->isLoaded() == false)
	{
		m_buildLabel->setText(tr("Build: Not loaded"));
	}
	else
	{
		QString buildPath = m_simulator->buildPath();
		m_buildLabel->setText(tr("Build: <a href=\"%1\">%1</a>").arg(buildPath));
	}

	fillEquipmentTree();

	return;
}

void SimProjectWidget::treeContextMenu(const QPoint& pos)
{
	QTreeWidgetItem* currentItem = m_treeWidget->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	QMenu menu(m_treeWidget);

	menu.addAction(m_openLmControlPageAction);
	menu.addAction(m_openLmCodePageAction);

	menu.exec(m_treeWidget->mapToGlobal(pos));
	return;
}

void SimProjectWidget::treeDoubleClicked(const QModelIndex& /*index*/)
{
	QTreeWidgetItem* currentItem = m_treeWidget->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	openModuleTabPage();
	return;
}

void SimProjectWidget::openModuleTabPage()
{
	QTreeWidgetItem* currentItem = m_treeWidget->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	// --
	//
	emit signal_openControlTabPage(lmEquipmentId);

	return;
}

void SimProjectWidget::openCodeTabPage()
{
	QTreeWidgetItem* currentItem = m_treeWidget->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	// --
	//
	emit signal_openCodeTabPage(lmEquipmentId);

	return;
}

void SimProjectWidget::updateModuleStates(Sim::ControlStatus state)
{
	Q_ASSERT(m_treeWidget);

	QString text;
	text.reserve(64);

	for (const auto& lmState : state.m_lmDeviceModes)
	{
		QList<QTreeWidgetItem*> items = m_treeWidget->findItems(lmState.lmEquipmentId, Qt::MatchFixedString | Qt::MatchRecursive, EquipmentTreeColumns::EquipmentID);

		for (QTreeWidgetItem* item : items)
		{
			QColor color{Qt::black};
			text.clear();

			if (state.m_state == Sim::SimControlState::Pause)
			{
				text = tr("Pause - ");
			}

			if (state.m_state != Sim::SimControlState::Stop)
			{
				switch (lmState.deviceMode)
				{
				case Sim::DeviceMode::Start:
					text += QStringLiteral("Start");
					break;
				case Sim::DeviceMode::Fault:
					text += QStringLiteral("Fault");
					color = qRgb(0xD0, 0x00, 0x00);
					break;
				case Sim::DeviceMode::Operate:
					text += QStringLiteral("Operate");
					break;
				default:
					Q_ASSERT(false);
					text += QStringLiteral("Unknown");
					color = qRgb(0xD0, 0x00, 0x00);
				}
			}

			item->setText(EquipmentTreeColumns::State, text);
			item->setTextColor(EquipmentTreeColumns::State, color);
		}
	}

	return;
}

void SimProjectWidget::fillEquipmentTree()
{
	assert(m_treeWidget);
	m_treeWidget->clear();

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Fill subsystems and modules
	//
	auto subsystems = m_simulator->subsystems();

	for (std::shared_ptr<Sim::Subsystem> ss : subsystems)
	{
		QStringList sl;
		sl << ss->subsystemId();

		QTreeWidgetItem* ssItem = new QTreeWidgetItem(m_treeWidget, sl);
		m_treeWidget->addTopLevelItem(ssItem);

		// Add LogicModules
		//
		auto logicModules = ss->logicModules();
		for (std::shared_ptr<Sim::LogicModule> lm : logicModules)
		{
			LogicModuleTreeItem* lmItem = new LogicModuleTreeItem{ssItem, lm};
			Q_UNUSED(lmItem);
		}

		m_treeWidget->expandItem(ssItem);
	}

	// Fill connections
	//
	QTreeWidgetItem* topConnectionItem = new QTreeWidgetItem(m_treeWidget, QStringList{} << tr("Connections"));
	m_treeWidget->addTopLevelItem(topConnectionItem);

	const Sim::Connections& connections = m_simulator->connections();
	std::vector<Sim::ConnectionPtr> ñonnectionList = connections.connections();

	for (const Sim::ConnectionPtr& c : ñonnectionList)
	{
		// Add Connection
		//
		ConnectionTreeItem* connItem = new ConnectionTreeItem(topConnectionItem, c);
		Q_UNUSED(connItem);

		// Ports are created in ConnectionTreeItem constructor
		//
	}

	m_treeWidget->expandItem(topConnectionItem);

	return;
}


namespace SimProjectTreeItems
{
	BaseTreeItem::BaseTreeItem(QTreeWidgetItem* parent, const QStringList& strings, int type) :
		QTreeWidgetItem(parent, strings, type)
	{
	}


	LogicModuleTreeItem::LogicModuleTreeItem(QTreeWidgetItem* parent, std::shared_ptr<Sim::LogicModule> lm) :
		BaseTreeItem(parent,
					 QStringList{} << lm->equipmentId()
								   << QString("n: %1, ch: %2").arg(lm->lmNumber()).arg(QChar('A' + static_cast<char>(lm->channel()))),
					 SimProjectTreeTypes::LogicModule),
		m_equipmentId(lm->equipmentId())
	{
		setData(0, Qt::UserRole, QVariant(m_equipmentId));
	}


	ConnectionTreeItem::ConnectionTreeItem(QTreeWidgetItem* parent, const Sim::ConnectionPtr& connection) :
		BaseTreeItem(parent,
					 QStringList{} << connection->connectionId()
								   << connection->type(),
					   SimProjectTreeTypes::Connection),
		m_connectionId(connection->connectionId())
	{
		setData(0, Qt::UserRole, QVariant(m_connectionId));

		// Create ports
		//
		const std::vector<Sim::ConnectionPortPtr>& ports = connection->ports();

		for (const Sim::ConnectionPortPtr& p : ports)
		{
			ConnectionPortTreeItem* connPortItem = new ConnectionPortTreeItem{this, p};
			Q_UNUSED(connPortItem);
		}
		return;
	}


	ConnectionPortTreeItem::ConnectionPortTreeItem(ConnectionTreeItem* parent, const Sim::ConnectionPortPtr& port) :
		BaseTreeItem(parent,
					 QStringList{} << port->portInfo().equipmentID
								   << QString::number(port->portInfo().portNo),
					 SimProjectTreeTypes::ConnectionPort),
		m_connectionPortId(port->portInfo().equipmentID)
	{
		setData(0, Qt::UserRole, QVariant(m_connectionPortId));
	}
}
