#include "SimProjectWidget.h"
#include "../Settings.h"


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

	m_equipmentTree = new QTreeWidget;
	m_equipmentTree->setUniformRowHeights(true);
	m_equipmentTree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	QStringList headerLabels;
	headerLabels << "ID";
	headerLabels << "Info";
	headerLabels << "State";
	m_equipmentTree->setHeaderLabels(headerLabels);

	QByteArray headerState = QSettings().value("SimulatorProjectWidget/headerState").toByteArray();
	m_equipmentTree->header()->restoreState(headerState);

	m_equipmentTree->clear();

	layout->addWidget(m_buildLabel);
	layout->addWidget(m_equipmentTree);

	setLayout(layout);

	// --
	//
	createActions();

	projectUpdated();	// Fill controls on start new window

	// --
	//
	connect(m_equipmentTree, &QTreeWidget::customContextMenuRequested, this, &SimProjectWidget::treeContextMenu);
	connect(m_equipmentTree, &QTreeWidget::doubleClicked, this, &SimProjectWidget::treeDoubleClicked);

	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimProjectWidget::projectUpdated);

	connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimProjectWidget::updateModuleStates);

	return;
}

SimProjectWidget::~SimProjectWidget()
{
	QByteArray headerState = m_equipmentTree->header()->saveState();
	QSettings().setValue("SimulatorProjectWidget/headerState", headerState);

	return;
}

void SimProjectWidget::createActions()
{
	m_openLmControlPageAction = new QAction(tr("Control Page..."), this);
	connect(m_openLmControlPageAction, &QAction::triggered, this, &SimProjectWidget::openControlTabPage);

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
	QTreeWidgetItem* currentItem = m_equipmentTree->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	QMenu menu(m_equipmentTree);

	menu.addAction(m_openLmControlPageAction);
	menu.addAction(m_openLmCodePageAction);

	menu.exec(m_equipmentTree->mapToGlobal(pos));
	return;
}

void SimProjectWidget::treeDoubleClicked(const QModelIndex& /*index*/)
{
	QTreeWidgetItem* currentItem = m_equipmentTree->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString lmEquipmentId = currentItem->data(0, Qt::UserRole).toString();
	if (lmEquipmentId.isEmpty() == true)
	{
		return;
	}

	openControlTabPage();
	return;
}

void SimProjectWidget::openControlTabPage()
{
	QTreeWidgetItem* currentItem = m_equipmentTree->currentItem();
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
	QTreeWidgetItem* currentItem = m_equipmentTree->currentItem();
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
	Q_ASSERT(m_equipmentTree);

	QString text;
	text.reserve(64);

	for (const auto& lmState : state.m_lmDeviceModes)
	{
		QList<QTreeWidgetItem*> items = m_equipmentTree->findItems(lmState.lmEquipmentId, Qt::MatchFixedString | Qt::MatchRecursive, EquipmentTreeColumns::EquipmentID);

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
	assert(m_equipmentTree);
	m_equipmentTree->clear();

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	auto subsystems = m_simulator->subsystems();

	for (std::shared_ptr<Sim::Subsystem> ss : subsystems)
	{
		QStringList sl;
		sl << ss->subsystemId();

		QTreeWidgetItem* ssItem = new QTreeWidgetItem(m_equipmentTree, sl);
		m_equipmentTree->addTopLevelItem(ssItem);

		// Add LogicModules
		//
		auto logicModules = ss->logicModules();
		for (std::shared_ptr<Sim::LogicModule> lm : logicModules)
		{
			QStringList slm;
			slm << lm->equipmentId();
			slm << QString("n: %1, ch: %2").arg(lm->lmNumber()).arg(QChar('A' + static_cast<char>(lm->channel())));

			QTreeWidgetItem* lmItem = new QTreeWidgetItem(ssItem, slm);
			lmItem->setData(0, Qt::UserRole, QVariant(lm->equipmentId()));
		}
	}

	m_equipmentTree->expandAll();
	return;
}
