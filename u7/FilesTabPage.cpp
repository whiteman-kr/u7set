#include "FilesTabPage.h"
#include "../include/DbStore.h"


//
//
//	FilesTabPage
//
//
FilesTabPage::FilesTabPage(DbStore* dbstore, QWidget* parent) :
	MainTabPage(dbstore, parent)
{
	assert(dbstore != nullptr);

	// Create Actions
	//
	CreateActions();

	//
	// Controls
	//
	m_filesView = new FileView(dbstore);

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	pLeftLayout->addWidget(m_filesView);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();

//	pRightLayout->addWidget(m_pNewProject);

	// Main Layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);

	setLayout(pMainLayout);

	// --
	//
	connect(dbStore(), &DbStore::projectOpened, this, &FilesTabPage::projectOpened);
	connect(dbStore(), &DbStore::projectClosed, this, &FilesTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

void FilesTabPage::CreateActions()
{
//	m_checkOutAction = new QAction(tr("Check Out"), this);
//	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
//	m_checkOutAction->setEnabled(false);
//	connect(m_checkOutAction, &QAction::triggered, this, &FilesTabPage::checkOutFiles);

	return;
}

void FilesTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void FilesTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

