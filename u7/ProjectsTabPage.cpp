#include "Stable.h"
#include "ProjectsTabPage.h"
#include "../include/DbController.h"
#include "CreateProjectDialog.h"
#include "LoginDialog.h"
#include "Settings.h"

ProjectsTabPage::ProjectsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent),
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
	headers.push_back(tr("Description"));
	headers.push_back(tr("Version"));

	m_pProjectTable->setColumnCount(3);
	m_pProjectTable->setHorizontalHeaderLabels(headers);
	m_pProjectTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_pProjectTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_pProjectTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

	connect(m_pProjectTable, &QTableWidget::itemSelectionChanged, this, &ProjectsTabPage::projectTableSelectionChanged);
	connect(m_pProjectTable, &QTableWidget::doubleClicked, this, &ProjectsTabPage::openProject);

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

	connect(m_pNewProject, &QPushButton::clicked, this, &ProjectsTabPage::createProject);
	connect(m_pOpenProject, &QPushButton::clicked, this, &ProjectsTabPage::openProject);
	connect(m_pCloseProject, &QPushButton::clicked, this, &ProjectsTabPage::closeProject);
	connect(m_pDeleteProject, &QPushButton::clicked, this, &ProjectsTabPage::deleteProject);
	connect(m_pRefreshProjectList, &QPushButton::clicked, this, &ProjectsTabPage::refreshProjectList);

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
	connect(dbController(), &DbController::projectOpened, this, &ProjectsTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &ProjectsTabPage::projectClosed);

	refreshProjectList();

	return;
}

void ProjectsTabPage::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	assert(m_pProjectTable);

	// Set ProjectTable colums width
	//
	m_pProjectTable->setColumnWidth(0, static_cast<int>(m_pProjectTable->size().width() * 0.30));
	m_pProjectTable->setColumnWidth(1, static_cast<int>(m_pProjectTable->size().width() * 0.60));

	return;
}

void ProjectsTabPage::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void ProjectsTabPage::projectOpened()
{
	refreshProjectList();

	m_pNewProject->setEnabled(false);
	m_pOpenProject->setEnabled(false);
	m_pCloseProject->setEnabled(true);
	m_pDeleteProject->setEnabled(false);
	m_pRefreshProjectList->setEnabled(true);
}

void ProjectsTabPage::projectClosed()
{
	refreshProjectList();

	m_pNewProject->setEnabled(true);
	m_pOpenProject->setEnabled(true);
	m_pCloseProject->setEnabled(false);
	m_pDeleteProject->setEnabled(true);
	m_pRefreshProjectList->setEnabled(true);
}

void ProjectsTabPage::createProject()
{
	CreateProjectDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		QString projectName = dialog.projectName;
		QString administratorPassword = dialog.adminstratorPassword;

		// Check "Project already exist"...
		//
		std::vector<DbProject> projects;
		dbController()->getProjectList(&projects, this);

		auto findPredicate = [&projectName](const DbProject& p) -> bool
			{
				return p.projectName() == projectName;
			};

		auto findResult = std::find_if(projects.begin(), projects.end(), findPredicate);

		if (findResult == projects.end())
		{
			// Add project
			//
			bool result = dbController()->createProject(projectName, administratorPassword, this);
			if (result == true)
			{
				dbController()->upgradeProject(projectName, administratorPassword, this);
			}

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

void ProjectsTabPage::openProject()
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

	if (dbController()->isProjectOpened() == true &&  dbController()->currentProject().projectName() == projectName)
	{
		QMessageBox::information(this, tr("Open project"), tr("Project %1 is already open.").arg(projectName));
		return;
	}

	if (dbController()->isProjectOpened() == true)
	{
		QMessageBox::information(this, tr("Open project"), tr("Another project is opened, please close it first."));
		return;
	}


	int projectVersion = m_pProjectTable->item(selectedItems[0]->row(), 2)->text().toInt();
	if (projectVersion > DbController::databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project version is higher than the programm version, please update software."));
		mb.exec();
		return;
	}

	if (projectVersion < DbController::databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project version is lower than the programm version, upgrade the project to the appropriate version?"));
		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Ok);

		int result = mb.exec();
		if (result == QMessageBox::Cancel)
		{
			return;
		}

		// Ask for Administartor's password
		//
		bool ok = false;

		QString password = QInputDialog::getText(this,
			tr("Upgrade project"),
			tr("Please, enter Administrator's password for project %1:").arg(projectName),
			QLineEdit::Password,
			QString(), &ok);

		if (ok == false)
		{
			return;
		}


		dbController()->upgradeProject(projectName, password, this);
		refreshProjectList();

		return;
	}

	bool exitLoginLoop = true;

	do
	{
		LoginDialog ld(theSettings.loginCompleter(), this);

		if (ld.exec() == QDialog::Accepted)
		{
			bool opened = dbController()->openProject(projectName, ld.username(), ld.password(), this);

			exitLoginLoop = opened;

			if (opened == true && theSettings.loginCompleter().contains(ld.username(), Qt::CaseSensitive) == false)
			{
				theSettings.loginCompleter() << ld.username();
			}
		}
		else
		{
			exitLoginLoop = true;
		}
	}
	while (exitLoginLoop == false);

	return;
}

void ProjectsTabPage::closeProject()
{
	if (dbController()->isProjectOpened() == false)
	{
		assert(dbController()->isProjectOpened() == true);
		return;
	}

	dbController()->closeProject(this);
	return;
}

void ProjectsTabPage::deleteProject()
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

	if (dbController()->isProjectOpened() == true)
	{
		QMessageBox msgBox;
		msgBox.setText(tr("You can not delete project while any is open. Please, close the project first."));
		msgBox.exec();
		return;
	}

	QMessageBox msgBox;
	msgBox.setText(tr("Deleting project %1.").arg(projectName));
	msgBox.setInformativeText(tr("Do you want to delete project %1 and lost all data?").arg(projectName));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	msgBox.setIcon(QMessageBox::Critical);
	int ret = msgBox.exec();

	if (ret == QMessageBox::Cancel)
	{
		return;
	}

	// Ask for Administrator's password
	//
	bool ok = false;

	QString password = QInputDialog::getText(this,
		tr("Delete project"),
		tr("Please, enter Administrator's password for project %1:").arg(projectName),
		QLineEdit::Password,
		QString(), &ok);

	if (ok == false)
	{
		return;
	}

	if (password.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("u7"), tr("Password cannot be empty!"));
		return;
	}

	dbController()->deleteProject(projectName, password, this);

	refreshProjectList();
	return;
}

void ProjectsTabPage::refreshProjectList()
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
	bool result = dbController()->getProjectList(&projects, this);

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
		m_pProjectTable->setItem(i, 1, new QTableWidgetItem(p.description()));
		m_pProjectTable->setItem(i, 2, new QTableWidgetItem(QString::number(p.version())));
		m_pProjectTable->item(i, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	}

	selectProject(selectedProject);

	return;
}

void ProjectsTabPage::selectProject(const QString& projectName)
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

void ProjectsTabPage::projectTableSelectionChanged()
{
	if (m_pProjectTable == nullptr)
	{
		assert(m_pProjectTable != nullptr);
		return;
	}

	if (dbController()->isProjectOpened())
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
