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
	BaseTreeItem* currentItem = dynamic_cast<BaseTreeItem*>(m_treeWidget->currentItem());
	if (currentItem == nullptr)
	{
		return;
	}

	currentItem->contextMenu(this, m_treeWidget->mapToGlobal(pos));

	return;
}

void SimProjectWidget::treeDoubleClicked(const QModelIndex& /*index*/)
{
	BaseTreeItem* currentItem = dynamic_cast<BaseTreeItem*>(m_treeWidget->currentItem());
	if (currentItem == nullptr)
	{
		return;
	}

	currentItem->doubleClick(this);

	return;
}


void SimProjectWidget::updateModuleStates(Sim::ControlStatus state)
{
	std::function<void(QTreeWidgetItem*)> visitTreeItems =
			[&visitTreeItems, &state, this](QTreeWidgetItem* treeItem)
			{
				for(int i = 0; i < treeItem->childCount(); i++)
				{
					QTreeWidgetItem* item = treeItem->child(i);

					if (BaseTreeItem* baseTreeItem = dynamic_cast<BaseTreeItem*>(item);
						baseTreeItem != nullptr)
					{
						baseTreeItem->updateState(this, state);
					}

					visitTreeItems(item);
				}
			};

	std::function<void(QTreeWidget*)> visitTopTreeItems =
			[&visitTreeItems, &state, this](QTreeWidget* treeWidget)
			{
				for(int i=0; i < treeWidget->topLevelItemCount(); i++)
				{
					QTreeWidgetItem* item = treeWidget->topLevelItem(i);

					if (BaseTreeItem* baseTreeItem = dynamic_cast<BaseTreeItem*>(item);
						baseTreeItem != nullptr)
					{
						baseTreeItem->updateState(this, state);
					}

					visitTreeItems(item);
				}
			};

	visitTopTreeItems(m_treeWidget);
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

	// Fill AppLogic Schemas
	//
	AppLogicSchemasTreeItem* appLogicSchemasTreeItem = new AppLogicSchemasTreeItem{nullptr};
	m_treeWidget->addTopLevelItem(appLogicSchemasTreeItem);

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
	std::vector<Sim::ConnectionPtr> connectionList = connections.connections();

	for (const Sim::ConnectionPtr& c : connectionList)
	{
		// Add Connection
		//
		ConnectionTreeItem* connItem = new ConnectionTreeItem(topConnectionItem, c);
		Q_UNUSED(connItem);

//		// Create ports
//		//
//		const std::vector<Sim::ConnectionPortPtr>& ports = c->ports();

//		for (const Sim::ConnectionPortPtr& p : ports)
//		{
//			ConnectionPortTreeItem* connPortItem = new ConnectionPortTreeItem{connItem, p};
//			Q_UNUSED(connPortItem);
//		}
	}

	m_treeWidget->expandItem(topConnectionItem);

	return;
}

const SimIdeSimulator* SimProjectWidget::simulator() const
{
	return m_simulator;
}

SimIdeSimulator* SimProjectWidget::simulator()
{
	return m_simulator;
}


namespace SimProjectTreeItems
{
	BaseTreeItem::BaseTreeItem(QTreeWidgetItem* parent,
							   const QStringList& strings) :
		QTreeWidgetItem(parent, strings, 0)
	{
	}

	void BaseTreeItem::updateState(SimProjectWidget* /*simProjectWidget*/, Sim::ControlStatus /*state*/)
	{
		return;
	}

	void BaseTreeItem::doubleClick(SimProjectWidget* /*simProjectWidget*/)
	{
		return;
	}

	void BaseTreeItem::contextMenu(SimProjectWidget* /*simProjectWidget*/, QPoint /*globalMousePos*/)
	{
		return;
	}


	LogicModuleTreeItem::LogicModuleTreeItem(QTreeWidgetItem* parent,
											 std::shared_ptr<Sim::LogicModule> lm) :
		BaseTreeItem(parent,
					 QStringList{} << lm->equipmentId()
								   << QString("n: %1, ch: %2").arg(lm->lmNumber()).arg(QChar('A' + static_cast<char>(lm->channel())))),
		m_equipmentId(lm->equipmentId())
	{
		setData(0, Qt::UserRole, QVariant(m_equipmentId));
		return;
	}

	void LogicModuleTreeItem::updateState(SimProjectWidget* /*simProjectWidget*/, Sim::ControlStatus state)
	{
		auto it = std::find_if(std::begin(state.m_lmDeviceModes),
							   std::end(state.m_lmDeviceModes),
							   [this](const Sim::ControlStatus::LmMode& p)
							   {
									return p.lmEquipmentId == this->m_equipmentId;
							   });

		if (it == std::end(state.m_lmDeviceModes))
		{
			return;
		}

		const Sim::ControlStatus::LmMode& lmState = *it;

		QColor color{Qt::black};

		QString text;
		text.reserve(64);

		if (state.m_state == Sim::SimControlState::Pause)
		{
			text = QObject::tr("Pause - ");
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

		this->setText(EquipmentTreeColumns::State, text);
		this->setTextColor(EquipmentTreeColumns::State, color);

		return;
	}

	void LogicModuleTreeItem::doubleClick(SimProjectWidget* simProjectWidget)
	{
		emit simProjectWidget->signal_openLogicModuleTabPage(m_equipmentId);
		return;
	}

	void LogicModuleTreeItem::contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos)
	{
		QMenu menu(this->treeWidget());

		menu.addAction(QObject::tr("Open..."),
			[simProjectWidget, this]()
			{
				emit simProjectWidget->signal_openLogicModuleTabPage(m_equipmentId);
			});

		menu.addAction(QObject::tr("Module Code..."),
			[simProjectWidget, this]()
			{
				emit simProjectWidget->signal_openCodeTabPage(m_equipmentId);
			});

		menu.exec(globalMousePos);
		return;
	}


	ConnectionTreeItem::ConnectionTreeItem(QTreeWidgetItem* parent,
										   const Sim::ConnectionPtr& connection) :
		BaseTreeItem(parent,
					 QStringList{} << connection->connectionId()
								   << connection->type()),
		m_connectionId(connection->connectionId())
	{
		setData(0, Qt::UserRole, QVariant(m_connectionId));

		const std::vector<Sim::ConnectionPortPtr>& ports = connection->ports();

		setToolTip(0, QObject::tr("ConnectionID: %1\n\tPort1: %2\n\tPort2: %3")
						.arg(m_connectionId)
						.arg(ports.size() >= 1 ? ports[0]->portInfo().equipmentID : "")
						.arg(ports.size() >= 2 ? ports[1]->portInfo().equipmentID : ""));

		return;
	}

	void ConnectionTreeItem::updateState(SimProjectWidget* simProjectWidget, Sim::ControlStatus state)
	{
		auto c = simProjectWidget->simulator()->connections().connection(m_connectionId);
		if (c == nullptr)
		{
			this->setText(EquipmentTreeColumns::State, {});
			return;
		}

		QString text;

		if (state.m_state == Sim::SimControlState::Stop)
		{
		}
		else
		{
			if (c->enabled() == true)
			{
				text = QObject::tr("ok");
			}
			else
			{
				text = QObject::tr("Disabled");
			}
		}

		this->setText(EquipmentTreeColumns::State, text);

		return;
	}

	void ConnectionTreeItem::doubleClick(SimProjectWidget* simProjectWidget)
	{
		emit simProjectWidget->signal_openConnectionTabPage(m_connectionId);
		return;
	}

	void ConnectionTreeItem::contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos)
	{
		auto c = simProjectWidget->simulator()->connections().connection(m_connectionId);
		if (c == nullptr)
		{
			return;
		}

		QMenu menu(this->treeWidget());

		menu.addAction(QObject::tr("Open..."),
			[simProjectWidget, this]()
			{
				emit simProjectWidget->signal_openConnectionTabPage(m_connectionId);
			});

		QAction* disableAction = menu.addAction(QObject::tr("Disable"),
			[simProjectWidget, connectionId = m_connectionId](bool checked)
			{
				simProjectWidget->simulator()->connections().disableConnection(connectionId, checked);
			});
		disableAction->setCheckable(true);
		disableAction->setChecked(!c->enabled());

		menu.exec(globalMousePos);
		return;
	}


//	ConnectionPortTreeItem::ConnectionPortTreeItem(ConnectionTreeItem* parent,
//												   const Sim::ConnectionPortPtr& port) :
//		BaseTreeItem(parent,
//					 QStringList{} << port->portInfo().equipmentID
//								   << QString::number(port->portInfo().portNo)),
//		m_connectionPortId(port->portInfo().equipmentID)
//	{
//		setData(0, Qt::UserRole, QVariant(m_connectionPortId));
//		return;
//	}

//	void ConnectionPortTreeItem::doubleClick(SimProjectWidget* simProjectWidget)
//	{
//		return;
//	}

	AppLogicSchemasTreeItem::AppLogicSchemasTreeItem(QTreeWidgetItem* parent) :
		BaseTreeItem(parent,
					 QStringList{} << "Application Logic Schemas")
	{
		QFont f = font(0);
		f.setWeight(QFont::Medium);

		setFont(0, f);

		return;
	}

	void AppLogicSchemasTreeItem::updateState(SimProjectWidget* /*simProjectWidget*/, Sim::ControlStatus /*state*/)
	{
	}

	void AppLogicSchemasTreeItem::doubleClick(SimProjectWidget* simProjectWidget)
	{
		emit simProjectWidget->signal_openAppSchemasTabPage();
		return;
	}

	void AppLogicSchemasTreeItem::contextMenu(SimProjectWidget* simProjectWidget, QPoint globalMousePos)
	{
		QMenu menu(this->treeWidget());

		menu.addAction(QObject::tr("Open..."),
			[simProjectWidget]()
			{
				emit simProjectWidget->signal_openAppSchemasTabPage();
			});

		menu.exec(globalMousePos);
		return;
	}
}
