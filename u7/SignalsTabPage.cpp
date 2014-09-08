#include "Stable.h"
#include "SignalsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"


//
//
// SignalsTabPage
//
//
SignalsTabPage::SignalsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	/*m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_equipmentView->addAction(m_addSystemAction);
	m_equipmentView->addAction(m_addCaseAction);
	m_equipmentView->addAction(m_addSubblockAction);
	m_equipmentView->addAction(m_addBlockAction);*/

	// Property View
	//
	//m_propertyView = new QTextEdit();

	// Splitter
	//
	//m_splitter = new QSplitter();

	//m_splitter->addWidget(m_equipmentView);
	//m_splitter->addWidget(m_propertyView);

	//m_splitter->setStretchFactor(0, 2);
	//m_splitter->setStretchFactor(1, 1);

	//m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	//
	// Layouts
	//

	QHBoxLayout* pMainLayout = new QHBoxLayout();

	//pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &SignalsTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &SignalsTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SignalsTabPage::~SignalsTabPage()
{
	//theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
	//theSettings.writeUserScope();
}

void SignalsTabPage::CreateActions()
{
	/*m_addSystemAction = new QAction(tr("Add System"), this);
	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
	//m_addSystemAction->setEnabled(false);
	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);
	*/
	return;
}

void SignalsTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void SignalsTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void SignalsTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}
