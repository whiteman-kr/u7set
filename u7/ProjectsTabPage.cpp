#include "ProjectsTabPage.h"
#include "../DbLib/DbController.h"
#include "CreateProjectDialog.h"
#include "LoginDialog.h"
#include "Settings.h"
#include "GlobalMessanger.h"

ProjectsTabPage::ProjectsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	//
	// Controls
	//
	m_projectTable = new QTableWidget();

	m_projectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_projectTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_projectTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_projectTable->setShowGrid(false);

	m_projectTable->verticalHeader()->hide();
	m_projectTable->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_projectTable->fontMetrics().height() * 1.4));
	m_projectTable->horizontalHeader()->setHighlightSections(false);

	m_projectTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_projectTable, &QWidget::customContextMenuRequested, this, &ProjectsTabPage::projectsContextMenuRequested);
	connect(m_projectTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &ProjectsTabPage::projectsSortIndicatorChanged);

	QStringList headers;
	headers.push_back(tr("Project Name"));
	headers.push_back(tr("Description"));
	headers.push_back(tr("Version"));

	m_projectTable->setColumnCount(3);
	m_projectTable->setHorizontalHeaderLabels(headers);
	m_projectTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_projectTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_projectTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

	connect(m_projectTable, &QTableWidget::itemSelectionChanged, this, &ProjectsTabPage::projectTableSelectionChanged);
	connect(m_projectTable, &QTableWidget::doubleClicked, this, &ProjectsTabPage::openProject);

	// Buttons
	//
	m_newProjectButton = new QPushButton(tr("New Project..."));
	m_openProjectButton = new QPushButton(tr("Open Project"));
	m_closeProjectButton = new QPushButton(tr("Close Project"));
	m_cloneProjectButton = new QPushButton(tr("Clone..."));
	m_deleteProjectButton = new QPushButton(tr("Delete Project"));
	m_refreshProjectListButton = new QPushButton(tr("Refresh"));

	m_openProjectButton->setEnabled(false);
	m_closeProjectButton->setEnabled(false);
	m_cloneProjectButton->setEnabled(false);
	m_deleteProjectButton->setEnabled(false);

	connect(m_newProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::createProject);
	connect(m_openProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::openProject);
	connect(m_closeProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::closeProject);
	connect(m_cloneProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::cloneProject);
	connect(m_deleteProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::deleteProject);
	connect(m_refreshProjectListButton, &QPushButton::clicked, this, &ProjectsTabPage::refreshProjectList);

	// Actions
	//
	m_newProjectAction = new QAction(tr("New Project..."), this);
	connect(m_newProjectAction, &QAction::triggered, this, &ProjectsTabPage::createProject);

	m_openProjectAction = new QAction(tr("Open Project..."), this);
	connect(m_openProjectAction, &QAction::triggered, this, &ProjectsTabPage::openProject);

	m_closeProjectAction = new QAction(tr("Close Project"), this);
	connect(m_closeProjectAction, &QAction::triggered, this, &ProjectsTabPage::closeProject);

	m_cloneProjectAction = new QAction(tr("Clone Project"), this);
	connect(m_cloneProjectAction, &QAction::triggered, this, &ProjectsTabPage::cloneProject);

	m_deleteProjectAction = new QAction(tr("Delete Project"), this);
	connect(m_deleteProjectAction, &QAction::triggered, this, &ProjectsTabPage::deleteProject);

	m_openProjectAction->setEnabled(false);
	m_closeProjectAction->setEnabled(false);
	m_cloneProjectAction->setEnabled(false);
	m_deleteProjectAction->setEnabled(false);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, this, &ProjectsTabPage::refreshProjectList);
	addAction(m_refreshAction);

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	pLeftLayout->addWidget(m_projectTable);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();

	pRightLayout->addWidget(m_newProjectButton);
	pRightLayout->addWidget(m_openProjectButton);
	pRightLayout->addWidget(m_closeProjectButton);
	pRightLayout->addWidget(m_refreshProjectListButton);
	pRightLayout->addStretch();
	pRightLayout->addWidget(m_cloneProjectButton);
	pRightLayout->addWidget(m_deleteProjectButton);

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

	assert(m_projectTable);

	// Set ProjectTable colums width
	//
	m_projectTable->setColumnWidth(0, static_cast<int>(m_projectTable->size().width() * 0.30));
	m_projectTable->setColumnWidth(1, static_cast<int>(m_projectTable->size().width() * 0.60));

	return;
}

void ProjectsTabPage::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

void ProjectsTabPage::projectOpened(DbProject project)
{
	refreshProjectList();

	m_newProjectButton->setEnabled(false);
	m_openProjectButton->setEnabled(false);
	m_closeProjectButton->setEnabled(true);
	m_cloneProjectButton->setEnabled(false);
	m_deleteProjectButton->setEnabled(false);

	m_refreshProjectListButton->setEnabled(true);

	m_newProjectAction->setEnabled(false);
	m_openProjectAction->setEnabled(false);
	m_closeProjectAction->setEnabled(true);
	m_cloneProjectAction->setEnabled(false);
	m_deleteProjectAction->setEnabled(false);

	GlobalMessanger::instance().fireProjectOpened(project);
	return;
}

void ProjectsTabPage::projectClosed()
{
	refreshProjectList();

	m_newProjectButton->setEnabled(true);
	m_openProjectButton->setEnabled(true);
	m_closeProjectButton->setEnabled(false);
	m_cloneProjectButton->setEnabled(true);
	m_deleteProjectButton->setEnabled(true);

	m_refreshProjectListButton->setEnabled(true);

	m_newProjectAction->setEnabled(true);
	m_openProjectAction->setEnabled(true);
	m_closeProjectAction->setEnabled(false);
	m_cloneProjectAction->setEnabled(true);
	m_deleteProjectAction->setEnabled(true);

	GlobalMessanger::instance().fireProjectClosed();
	return;
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
				return p.projectName().compare(projectName, Qt::CaseInsensitive) == 0;
			};

		auto findResult = std::find_if(projects.begin(), projects.end(), findPredicate);

		if (findResult == projects.end())
		{
			// Add project
			//
			if (bool result = dbController()->createProject(projectName, administratorPassword, this);
				result == true)
			{
				bool upgradeOk = dbController()->upgradeProject(projectName, administratorPassword, true, this);

				if (upgradeOk == true)
				{
					// Open project to write Description and UppercaseAppSignalID properties
					//
					result = dbController()->openProject(projectName, "Administrator", administratorPassword, this);

					if (result == true)
					{
						dbController()->setProjectProperty(Db::ProjectProperty::Description, dialog.projectDescription, this);
						dbController()->setProjectProperty(Db::ProjectProperty::SafetyProject, true, this);
						dbController()->setProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, true, this);
						dbController()->closeProject(this);
					}
				}
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
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();
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


	int projectVersion = m_projectTable->item(selectedItems[0]->row(), 2)->text().toInt();
	if (projectVersion > DbController::databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project database version is higher than the program version. Please update software."));
		mb.setFixedSize(mb.minimumSizeHint());
		mb.exec();
		return;
	}

	if (projectVersion < DbController::databaseVersion())
	{
		QMessageBox mb(this);

		mb.setText(tr("You cannot open this project."));
		mb.setInformativeText(tr("The project database version is lower than the program version. Do you want to upgrade the project to the appropriate version?"));
		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Ok);
		mb.setFixedSize(mb.minimumSizeHint());

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
			tr("Please, enter <b>Administrator</b>'s password for project %1:").arg(projectName),
			QLineEdit::Password,
			QString(), &ok, Qt::MSWindowsFixedSizeDialogHint);

		if (ok == false)
		{
			return;
		}

		dbController()->upgradeProject(projectName, password, false, this);
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

	emit projectAboutToBeClosed();
	dbController()->closeProject(this);
	return;
}

void ProjectsTabPage::cloneProject()
{
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();
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
		QMessageBox msgBox(this);
		msgBox.setText(tr("You can not clone project while any is open. Please, close the project first."));
		msgBox.exec();
		return;
	}

	// Ask for Administrator's password
	//
	bool ok = false;

	QString password = QInputDialog::getText(this,
		tr("Clone project"),
		tr("Please, enter <b>Administrator</b>'s password for project <b>%1</b>:").arg(projectName),
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

	// GetNewProject name and description
	//
	QString newProjectName = QInputDialog::getText(this,
		tr("Clone project"),
		tr("Please, enter new project name:"),
		QLineEdit::Normal,
		"cloned_" + projectName, &ok);

	if (ok == false)
	{
		return;
	}

	newProjectName = newProjectName.trimmed();

	if (newProjectName.isEmpty() == true)
	{
		QMessageBox::critical(this, tr("u7"), tr("Project name cannot be empty!"));
		return;
	}

	// Clone
	//
	dbController()->cloneProject(projectName, password, newProjectName, this);

	refreshProjectList();
	return;

}

void ProjectsTabPage::deleteProject()
{
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();
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
		QMessageBox msgBox(this);
		msgBox.setText(tr("You can not delete project while any is open. Please, close the project first."));
		msgBox.exec();
		return;
	}

	QMessageBox msgBox(this);
	msgBox.setText(tr("Deleting project %1.").arg(projectName));
	msgBox.setInformativeText(tr("Do you want to <b>delete project %1</b> and discard all data?").arg(projectName));
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
		tr("Please, enter <b>Administrator</b>'s password for project <b>%1</b>:").arg(projectName),
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

	dbController()->deleteProject(projectName, password, false, this);

	refreshProjectList();
	return;
}

void ProjectsTabPage::refreshProjectList()
{
	assert(m_projectTable != nullptr);

	// Save current selection
	//
	QString selectedProject;
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();

	if (selectedItems.size() != 0 && selectedItems[0]->column() == 0)
	{
		selectedProject = selectedItems[0]->text();
	}

	// clear all records
	//
	m_projectTable->setRowCount(0);

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
	m_projectTable->setRowCount(static_cast<int>(projects.size()));

	m_projectTable->setSortingEnabled(false);

	for (unsigned int i = 0; i < projects.size(); i++)
	{
		const DbProject& p = projects[i];

		m_projectTable->setItem(i, 0, new QTableWidgetItem(p.projectName()));
		m_projectTable->setItem(i, 1, new QTableWidgetItem(p.description()));

		QTableWidgetItem* itemVersion = new QTableWidgetItem(QString::number(p.version()));
		itemVersion->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		m_projectTable->setItem(i, 2, itemVersion);
	}

	// Sort projects
	//
	m_projectTable->setSortingEnabled(true);
	m_projectTable->sortByColumn(theSettings.m_projectsSortColumn, theSettings.m_projectsSortOrder);

	selectProject(selectedProject);

	return;
}

void ProjectsTabPage::selectProject(const QString& projectName)
{
	assert(m_projectTable != nullptr);

	// --
	//
	QString lcProjectName = projectName.toLower();

	for (int i = 0; i < m_projectTable->rowCount(); i++)
	{
		QTableWidgetItem* item = m_projectTable->item(i, 0);
		assert(item != nullptr);

		if (item != nullptr &&
			item->text().toLower() == lcProjectName)
		{
			m_projectTable->setCurrentCell(i, 0);
			break;
		}
	}

	return;
}

void ProjectsTabPage::projectsContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	QMenu menu(this);

	menu.addAction(m_newProjectAction);
	menu.addAction(m_openProjectAction);
	menu.addAction(m_closeProjectAction);
	menu.addSeparator();
	menu.addAction(m_cloneProjectAction);
	menu.addAction(m_deleteProjectAction);
	menu.addSeparator();
	menu.addAction(m_refreshAction);

	menu.exec(QCursor::pos());
	return;
}

void ProjectsTabPage::projectsSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_projectsSortColumn = column;
	theSettings.m_projectsSortOrder = order;
}

void ProjectsTabPage::projectTableSelectionChanged()
{
	if (m_projectTable == nullptr)
	{
		assert(m_projectTable != nullptr);
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
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();

	if (selectedItems.isEmpty() == false)
	{
		m_openProjectButton->setEnabled(true);
		m_cloneProjectButton->setEnabled(true);
		m_deleteProjectButton->setEnabled(true);

		m_openProjectAction->setEnabled(true);
		m_cloneProjectAction->setEnabled(true);
		m_deleteProjectAction->setEnabled(true);
	}
	else
	{
		m_openProjectButton->setEnabled(false);
		m_cloneProjectButton->setEnabled(false);
		m_deleteProjectButton->setEnabled(false);

		m_openProjectAction->setEnabled(true);
		m_cloneProjectAction->setEnabled(true);
		m_deleteProjectAction->setEnabled(true);
	}

	return;
}
