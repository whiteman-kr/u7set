#include "SimulatorProjectWidget.h"
#include <QVBoxLayout>
#include <QMenu>
#include "../Settings.h"

SimulatorProjectWidget::SimulatorProjectWidget(std::shared_ptr<SimIdeSimulator> simulator, QWidget* parent) :
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
	connect(m_equipmentTree, &QTreeWidget::customContextMenuRequested, this, &SimulatorProjectWidget::treeContextMenu);
	connect(m_equipmentTree, &QTreeWidget::doubleClicked, this, &SimulatorProjectWidget::treeDoubleClicked);

	connect(m_simulator.get(), &Sim::Simulator::projectUpdated, this, &SimulatorProjectWidget::projectUpdated);

	return;
}

SimulatorProjectWidget::~SimulatorProjectWidget()
{
	QByteArray headerState = m_equipmentTree->header()->saveState();
	QSettings().setValue("SimulatorProjectWidget/headerState", headerState);

	return;
}

void SimulatorProjectWidget::createActions()
{
	m_openLmControlPageAction = new QAction(tr("Control Page..."));
	connect(m_openLmControlPageAction, &QAction::triggered, this, &SimulatorProjectWidget::openControlTabPage);

	return;
}

void SimulatorProjectWidget::projectUpdated()
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

void SimulatorProjectWidget::treeContextMenu(const QPoint& pos)
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

	menu.exec(m_equipmentTree->mapToGlobal(pos));
	return;
}

void SimulatorProjectWidget::treeDoubleClicked(const QModelIndex& /*index*/)
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

void SimulatorProjectWidget::openControlTabPage()
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

void SimulatorProjectWidget::fillEquipmentTree()
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
