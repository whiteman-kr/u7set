#include "Stable.h"
#include "DatabaseTabPage.h"
#include "../include/DbStore.h"
#include "CreateProjectDialog.h"
#include "LoginDialog.h"

DatabaseTabPage::DatabaseTabPage(DbStore* dbstore, QWidget* parent) :
	MainTabPage(dbstore, parent),
	m_pProjectTable(nullptr),
	m_pNewProject(nullptr),
	m_pOpenProject(nullptr),
	m_pCloseProject(nullptr),
	m_pDeleteProject(nullptr),
	m_pRefreshProjectList(nullptr)
{
	//
	// Controls
	//
	m_pProjectTable = new QTableWidget();

	m_pProjectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pProjectTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_pProjectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_pProjectTable->setShowGrid(false);

	m_pProjectTable->verticalHeader()->hide();
	m_pProjectTable->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_pProjectTable->fontMetrics().height() * 1.4));
	m_pProjectTable->horizontalHeader()->setHighlightSections(false);

	QStringList headers;
	headers.push_back(tr("Project Name"));
	headers.push_back(tr("Version"));

	m_pProjectTable->setColumnCount(2);
	m_pProjectTable->setHorizontalHeaderLabels(headers);
	m_pProjectTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_pProjectTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);

	connect(m_pProjectTable, &QTableWidget::itemSelectionChanged, this, &DatabaseTabPage::projectTableSelectionChanged);
	connect(m_pProjectTable, &QTableWidget::doubleClicked, this, &DatabaseTabPage::openProject);

	// Buttons
	//
	m_pNewProject = new QPushButton(tr("New Project..."));
	m_pOpenProject = new QPushButton(tr("Open Project"));
	m_pCloseProject = new QPushButton(tr("Close Project"));
	m_pDeleteProject = new QPushButton(tr("Delete Project"));
	m_pRefreshProjectList = new QPushButton(tr("Refresh"));

	m_pOpenProject->setEnabled(false);
	m_pCloseProject->setEnabled(false);
	m_pDeleteProject->setEnabled(false);

	connect(m_pNewProject, &QPushButton::clicked, this, &DatabaseTabPage::createProject);
	connect(m_pOpenProject, &QPushButton::clicked, this, &DatabaseTabPage::openProject);
	connect(m_pCloseProject, &QPushButton::clicked, this, &DatabaseTabPage::closeProject);
	connect(m_pRefreshProjectList, &QPushButton::clicked, this, &DatabaseTabPage::refreshProjectList);

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	pLeftLayout->addWidget(m_pProjectTable);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();

	pRightLayout->addWidget(m_pNewProject);
	pRightLayout->addWidget(m_pOpenProject);
	pRightLayout->addWidget(m_pCloseProject);
	pRightLayout->addStretch();
	pRightLayout->addWidget(m_pDeleteProject);
	pRightLayout->addWidget(m_pRefreshProjectList);

	// Main Layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);

	setLayout(pMainLayout);

	// --
	//
	connect(dbStore(), &DbStore::projectOpened, this, &DatabaseTabPage::projectOpened);
	connect(dbStore(), &DbStore::projectClosed, this, &DatabaseTabPage::projectClosed);

	// --
	//
	refreshProjectList();

	return;
}

void DatabaseTabPage::projectOpened()
{
	refreshProjectList();

	m_pNewProject->setEnabled(false);
	m_pOpenProject->setEnabled(false);
	m_pCloseProject->setEnabled(true);
	m_pDeleteProject->setEnabled(false);
	m_pRefreshProjectList->setEnabled(true);
}

void DatabaseTabPage::projectClosed()
{
	refreshProjectList();

	m_pNewProject->setEnabled(true);
	m_pOpenProject->setEnabled(true);
	m_pCloseProject->setEnabled(false);
	m_pDeleteProject->setEnabled(true);
	m_pRefreshProjectList->setEnabled(true);
}

void DatabaseTabPage::createProject()
{
	CreateProjectDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		QString projectName = dialog.projectName;
		QString administratorPassword = dialog.adminstratorPassword;

		// Check "Project already exist"...
		//
		std::vector<DbProject> projects;
		dbStore()->getProjectList(projects);

		auto findPredicate = [&projectName](const DbProject& p) -> bool
			{
				return p.projectName() == projectName;
			};

		auto findResult = std::find_if(projects.begin(), projects.end(), findPredicate);

		if (findResult == projects.end())
		{
			// Add project
			//
			dbStore()->createProject(projectName, administratorPassword);

			refreshProjectList();
			selectProject(projectName);
		}
		else
		{
			QMessageBox mb(this);
			mb.setText(tr("Database %1 already exists.").arg(projectName));

			mb.exec();
		}
	}

	return;
}

void DatabaseTabPage::openProject()
{
	QList<QTableWidgetItem*> selectedItems = m_pProjectTable->selectedItems();

	if (selectedItems.size() == 0 || selectedItems[0]->column() != 0)
	{
		return;
	}

	QString projectName = selectedItems[0]->text();
	if (projectName.isEmpty())
	{
		return;
	}

	if (dbStore()->isProjectOpened() == true &&  dbStore()->currentProject().projectName() == projectName)
	{
		QMessageBox::information(this, tr("Open project"), tr("Project %1 is already open.").arg(projectName));
		return;
	}

	if (dbStore()->isProjectOpened() == true)
	{
		QMessageBox::information(this, tr("Open project"), tr("Another project is opened, please close it first."));
		return;
	}


	int projectVersion = m_pProjectTable->item(selectedItems[0]->row(), 1)->text().toInt();
	if (projectVersion > dbStore()->databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project version is higher than the programm version, please update software."));
		mb.exec();
		return;
	}

	if (projectVersion < dbStore()->databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project version is lower than the programm version, upgrade the project to the appropriate version?"));
		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Ok);

		int result = mb.exec();
		if (result == QMessageBox::Ok)
		{
			dbStore()->upgradeProject(projectName, this);
			refreshProjectList();
		}

		return;
	}

	LoginDialog ld(this);

	if (ld.exec() == QDialog::Accepted)
	{
		dbStore()->openProject(projectName, ld.username(), ld.password());
	}

	return;
}

void DatabaseTabPage::closeProject()
{
	if (dbStore()->isProjectOpened() == false)
	{
		assert(dbStore()->isProjectOpened() == true);
		return;
	}

	dbStore()->closeProject();
	return;
}

void DatabaseTabPage::refreshProjectList()
{
	assert(m_pProjectTable != nullptr);

	// Save current selection
	//
	QString selectedProject;
	QList<QTableWidgetItem*> selectedItems = m_pProjectTable->selectedItems();

	if (selectedItems.size() != 0 && selectedItems[0]->column() == 0)
	{
		selectedProject = selectedItems[0]->text();
	}

	// clear all records
	//
	m_pProjectTable->setRowCount(0);

	// Get project list from database (synchronous call)
	//
	std::vector<DbProject> projects;
	bool result = dbStore()->getProjectList(projects);

	if (result == false)
	{
		return;
	}

	// Fill the project list with the received values
	//
	m_pProjectTable->setRowCount(static_cast<int>(projects.size()));

	for (unsigned int i = 0; i < projects.size(); i++)
	{
		const DbProject& p = projects[i];

		m_pProjectTable->setItem(i, 0, new QTableWidgetItem(p.projectName()));
		m_pProjectTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.version())));
		m_pProjectTable->item(i, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}

	selectProject(selectedProject);

	return;
}

void DatabaseTabPage::selectProject(const QString& projectName)
{
	assert(m_pProjectTable != nullptr);

	// --
	//
	QString lcProjectName = projectName.toLower();

	for (int i = 0; i < m_pProjectTable->rowCount(); i++)
	{
		QTableWidgetItem* item = m_pProjectTable->item(i, 0);

		assert(item != nullptr);

		if (item->text().toLower() == lcProjectName)
		{
			m_pProjectTable->setCurrentCell(i, 0);
			break;
		}
	}

	return;
}

void DatabaseTabPage::projectTableSelectionChanged()
{
	if (m_pProjectTable == nullptr)
	{
		assert(m_pProjectTable != nullptr);
		return;
	}

	if (dbStore()->isProjectOpened())
	{
		// Can just close the project, it was set in projectOpened slot
		//
		return;
	}

	// Project is closed, so we can open project if the list has selected row
	//
	QList<QTableWidgetItem*> selectedItems = m_pProjectTable->selectedItems();

	if (selectedItems.isEmpty() == false)
	{
		m_pOpenProject->setEnabled(true);
		m_pDeleteProject->setEnabled(true);
	}
	else
	{
		m_pOpenProject->setEnabled(false);
		m_pDeleteProject->setEnabled(false);
	}

	return;
}
