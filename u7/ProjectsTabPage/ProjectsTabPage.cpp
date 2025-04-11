#include "ProjectsTabPage.h"
#include "../AppSettings.h"
#include "../GlobalMessanger.h"
#include "../Settings.h"
#include "CreateProjectDialog.h"
#include "LoginDialog.h"
#include "ProjectBackup.h"


ProjectsTabPage::ProjectsTabPage(DbController* dbcontroller, std::function<bool(void)> preCloseConditionsCallback, QWidget* parent) :
	MainTabPage(dbcontroller, parent),
	m_preCloseConditionsCallback(std::move(preCloseConditionsCallback))
{
	assert(m_preCloseConditionsCallback);

	//
	// Controls
	//
	m_projectTable = new QTableWidget{};

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
	m_newProjectButton = new QPushButton{tr("New Project...")};
	m_openProjectButton = new QPushButton{tr("Open Project")};
	m_closeProjectButton = new QPushButton{tr("Close Project")};
	m_backupProjectButton = new QPushButton{tr("Backup")};
	m_restoreProjectButton = new QPushButton{tr("Restore...")};
	m_cloneProjectButton = new QPushButton{tr("Clone...")};
	m_deleteProjectButton = new QPushButton{tr("Delete Project")};
	m_refreshProjectListButton = new QPushButton{tr("Refresh")};

	m_openProjectButton->setEnabled(false);
	m_closeProjectButton->setEnabled(false);
	m_cloneProjectButton->setEnabled(false);
	m_deleteProjectButton->setEnabled(false);

	connect(m_newProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::createProject);
	connect(m_openProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::openProject);
	connect(m_closeProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::closeProject);

	connect(m_backupProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::backupProject);
	connect(m_restoreProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::restoreProject);

	connect(m_cloneProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::cloneProject);
	connect(m_deleteProjectButton, &QPushButton::clicked, this, &ProjectsTabPage::deleteProject);

	connect(m_refreshProjectListButton, &QPushButton::clicked, this, &ProjectsTabPage::refreshProjectList);

	// Actions
	//
	m_newProjectAction = new QAction{tr("New Project..."), this};
	connect(m_newProjectAction, &QAction::triggered, this, &ProjectsTabPage::createProject);

	m_openProjectAction = new QAction{tr("Open Project..."), this};
	connect(m_openProjectAction, &QAction::triggered, this, &ProjectsTabPage::openProject);

	m_closeProjectAction = new QAction{tr("Close Project"), this};
	connect(m_closeProjectAction, &QAction::triggered, this, &ProjectsTabPage::closeProject);

	m_backupProjectAction = new QAction{tr("Backup"), this};
	connect(m_backupProjectAction, &QAction::triggered, this, &ProjectsTabPage::backupProject);

	m_restoreProjectAction = new QAction{tr("Restore..."), this};
	connect(m_restoreProjectAction, &QAction::triggered, this, &ProjectsTabPage::restoreProject);

	m_cloneProjectAction = new QAction{tr("Clone Project"), this};
	connect(m_cloneProjectAction, &QAction::triggered, this, &ProjectsTabPage::cloneProject);

	m_deleteProjectAction = new QAction{tr("Delete Project"), this};
	connect(m_deleteProjectAction, &QAction::triggered, this, &ProjectsTabPage::deleteProject);

	m_refreshAction = new QAction{tr("Refresh"), this};
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, this, &ProjectsTabPage::refreshProjectList);
	addAction(m_refreshAction);

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout{};
	pLeftLayout->addWidget(m_projectTable);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout{};

	pRightLayout->addWidget(m_newProjectButton);
	pRightLayout->addWidget(m_openProjectButton);
	pRightLayout->addWidget(m_closeProjectButton);
	pRightLayout->addWidget(m_refreshProjectListButton);
	pRightLayout->addStretch();
	pRightLayout->addWidget(m_backupProjectButton);
	pRightLayout->addWidget(m_restoreProjectButton);
	pRightLayout->addWidget(m_cloneProjectButton);
	pRightLayout->addWidget(m_deleteProjectButton);

	// Main Layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout{};
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &ProjectsTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &ProjectsTabPage::projectClosed);

	updateUiState(false);

	return;
}

void ProjectsTabPage::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	assert(m_projectTable);

	// Set ProjectTable columns width
	//
	m_projectTable->setColumnWidth(0, static_cast<int>(m_projectTable->size().width() * 0.30));
	m_projectTable->setColumnWidth(1, static_cast<int>(m_projectTable->size().width() * 0.60));

	return;
}

void ProjectsTabPage::projectOpened(DbProject project)
{
	refreshProjectList();
	updateUiState(true);

	GlobalMessanger::instance().fireProjectOpened(project);
	return;
}

void ProjectsTabPage::projectClosed()
{
	refreshProjectList();
	updateUiState(false);

	GlobalMessanger::instance().fireProjectClosed();
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

void ProjectsTabPage::updateUiState(bool isOpened)
{
	bool backupIsPossible = theAppSettings.pgDumpCommand().isEmpty() == false;
	bool restoreIsPossible = theAppSettings.psqlCommand().isEmpty() == false;

	bool isClosed = !isOpened;
	bool selected = m_projectTable && m_projectTable->selectedItems().isEmpty() == false;

	m_newProjectButton->setEnabled(isClosed);
	m_openProjectButton->setEnabled(isClosed && selected);
	m_closeProjectButton->setEnabled(isOpened);

	m_backupProjectButton->setEnabled(isClosed && backupIsPossible && selected);
	m_restoreProjectButton->setEnabled(isClosed && restoreIsPossible);

	m_cloneProjectButton->setEnabled(isClosed && selected);
	m_deleteProjectButton->setEnabled(isClosed && selected);

	m_refreshProjectListButton->setEnabled(true);

	m_newProjectAction->setEnabled(isClosed);
	m_openProjectAction->setEnabled(isClosed && selected);
	m_closeProjectAction->setEnabled(isOpened);
	m_backupProjectAction->setEnabled(isClosed && backupIsPossible && selected);
	m_restoreProjectAction->setEnabled(isClosed && restoreIsPossible);
	m_cloneProjectAction->setEnabled(isClosed && selected);
	m_deleteProjectAction->setEnabled(isClosed && selected);

	return;
}

void ProjectsTabPage::createProject()
{
	// Use different DbController as during cretion project it can be opened and closed,
	// and it must not emit any siganls to existing UI.
	//
	DbController dbc;

	dbc.enableProgress();
	dbc.setHost(theAppSettings.serverHost());
	dbc.setPort(theAppSettings.serverPort());
	dbc.setServerUsername(theAppSettings.serverUsername());
	dbc.setServerPassword(theAppSettings.serverPassword());

	CreateProjectDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		QString projectName = dialog.projectName;
		QString administratorPassword = dialog.adminstratorPassword;

		// Check "Project already exist"...
		//
		std::vector<DbProject> projects;
		dbc.getProjectList(&projects, this);

		auto findPredicate = [&projectName](const DbProject& p) -> bool
		{
			return p.projectName().compare(projectName, Qt::CaseInsensitive) == 0;
		};

		auto findResult = std::find_if(projects.begin(), projects.end(), findPredicate);

		if (findResult == projects.end())
		{
			// Add project
			//
			if (bool result = dbc.createProject(projectName, administratorPassword, this); result == true)
			{
				bool upgradeOk = dbc.upgradeProject(projectName, administratorPassword, true, this);

				if (upgradeOk == true)
				{
					// Open project to write Description and UppercaseAppSignalID properties
					//
					result = dbc.openProject(projectName, "Administrator", administratorPassword, this);

					if (result == true)
					{
						dbc.setProjectProperty(Db::ProjectProperty::Description, dialog.projectDescription, this);
						dbc.setProjectProperty(Db::ProjectProperty::SafetyProject, true, this);
						dbc.setProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, true, this);
						dbc.closeProject(this);
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

	if (dbController()->isProjectOpened() == true && dbController()->currentProject().projectName() == projectName)
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
		mb.setInformativeText(
			tr("The project database version (%1) is higher than the supported project version %2.\n\nPlease update software.")
				.arg(projectVersion)
				.arg(DbController::databaseVersion()));
		mb.setFixedSize(mb.minimumSizeHint());
		mb.exec();
		return;
	}

	if (projectVersion < DbController::databaseVersion())
	{
		QMessageBox mb{this};

		mb.setTextFormat(Qt::RichText);
		mb.setText(
			tr("<font color='red'>Do you want to upgrade the project to the version %1?</font>").arg(DbController::databaseVersion()));
		mb.setInformativeText(tr("The project database version (%1) is lower than the supported version %2.")
								  .arg(projectVersion)
								  .arg(DbController::databaseVersion()));
		mb.setDetailedText(tr("During the upgrade to a newer version a project database backup will be created, it will have a name like "
							  "u7upgrade%1_%2_[date_and_time_of_upgrade].\n\nThe database administrator can restore the backup by renaming "
							  "it to u7_[new_name].")
							   .arg(projectVersion)
							   .arg(projectName));

		mb.setIcon(QMessageBox::Warning);
		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Cancel);

		if (int result = mb.exec(); result == QMessageBox::Cancel)
		{
			return;
		}

		// Ask for Administrator's password
		//
		bool ok = false;

		QString password = QInputDialog::getText(this,
												 tr("Upgrade project"),
												 tr("Please, enter <b>Administrator</b>'s password for project %1:").arg(projectName),
												 QLineEdit::Password,
												 QString(),
												 &ok,
												 Qt::MSWindowsFixedSizeDialogHint);

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
	} while (exitLoginLoop == false);

	return;
}

void ProjectsTabPage::backupProject()
{
	QList<QTableWidgetItem*> selectedItems = m_projectTable->selectedItems();
	if (selectedItems.size() == 0 || selectedItems[0]->column() != 0)
	{
		return;
	}

	QString project = selectedItems[0]->text();
	if (project.isEmpty())
	{
		return;
	}

	QString db = "u7_" + project;

	ProjectBackup backuper;
	ProjectBackup::Server server{theAppSettings.serverHost(),
								 theAppSettings.serverPort(),
								 theAppSettings.serverUsername(),
								 theAppSettings.serverPassword()};

	backuper.setBackupExecutable(theAppSettings.pgDumpCommand());

	if (backuper.canBackup() == false)
	{
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save Backup As"),
													QString{"%1.sqlbackup"}.arg(db),
													tr("Backup files (*.sqlbackup);;All files (*.*)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	QString tempFileName = fileName + ".tmp";

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QString error;
	std::atomic<bool> abort = false;

	auto backupFunc = [&backuper, &error, &abort](QString db, QString fileName, const ProjectBackup::Server& server) -> bool
	{
		return backuper.backup(db, fileName, server, error, abort);
	};

	auto future = QtConcurrent::run(backupFunc, db, tempFileName, server);

	{
		QThread::msleep(200);

		QProgressDialog progress("Backing up...", "Abort", 0, INT_MAX, this);
		progress.setMinimumDuration(0);
		progress.setWindowModality(Qt::WindowModal);

		while (future.isFinished() == false)
		{
			QFile file{tempFileName};
			if (file.open(QFile::ReadOnly) == true)
			{
				qint64 fileSize = file.size();
				qint64 fileSizeMb = fileSize / (qint64)(1024 * 1024); // File size in megabytes.
				qDebug() << fileSizeMb;
				progress.setValue(fileSizeMb);
				progress.setLabelText(QString("Backing up: %1 MB").arg(fileSizeMb));
			}

			for (int i = 0; i < 10; i++)
			{
				if (progress.wasCanceled() == true)
				{
					abort.store(true);
				}

				QApplication::processEvents();
				QThread::msleep(10);
			}
		}
	}

	QApplication::restoreOverrideCursor();

	if (future.result() == false)
	{
		// Delete tempFileName
		//
		QFile::remove(tempFileName);

		QMessageBox mb{this};
		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setText(tr("Backup failed."));
		mb.setInformativeText(error.left(511));
		mb.exec();
	}
	else
	{
		// Rename tempFileName to fileName
		//
		QFile::remove(fileName); // In case such file already exists.
		bool renameOk = QFile::rename(tempFileName, fileName);

		if (renameOk == false)
		{
			QMessageBox::critical(this, qAppName(), QString{"File %1 write error."}.arg(fileName));
		}
		else
		{
			QMessageBox mb{this};
			mb.setIcon(QMessageBox::Icon::Information);
			mb.setText(tr("Backup completed successfully."));
			mb.addButton(QMessageBox::Ok);
			auto button = mb.addButton(tr("Open Folder"), QMessageBox::ActionRole);

			connect(button,
					&QPushButton::clicked,
					[fileName]()
					{
						QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(fileName).absolutePath()));
					});

			mb.exec();
		}
	}

	return;
}

void ProjectsTabPage::restoreProject()
{
	ProjectBackup restorer;
	restorer.setRestoreExecutable(theAppSettings.psqlCommand());

	ProjectBackup::Server server{theAppSettings.serverHost(),
								 theAppSettings.serverPort(),
								 theAppSettings.serverUsername(),
								 theAppSettings.serverPassword()};

	if (restorer.canRestore() == false)
	{
		return;
	}

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), {}, tr("Backup files (*.sqlbackup);;All files (*.*)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	QString supposedProjectName = QFileInfo(fileName).baseName();
	if (supposedProjectName.startsWith("u7_", Qt::CaseInsensitive) == true)
	{
		supposedProjectName.remove(0, 3);
	}

	// Ask name for the restored database
	//
	std::vector<DbProject> projects;
	bool result = dbController()->getProjectList(&projects, this);

	if (result == false)
	{
		QMessageBox::critical(this, tr("Restore project"), tr("Failed to get project list."));
		return;
	}

	QString projectName;

	while (true)
	{
		bool projectNameOk = false;
		projectName = QInputDialog::getText(this,
											tr("Restore project"),
											tr("Please, enter project name:"),
											QLineEdit::Normal,
											supposedProjectName,
											&projectNameOk);

		if (projectNameOk == false)
		{
			// User pressed Cancel.
			//
			return;
		}

		projectName = projectName.trimmed();

		if (projectName.isEmpty() == true)
		{
			QMessageBox::critical(this, tr("u7"), tr("Project name cannot be empty!"));
			supposedProjectName = projectName;
			continue;
		}

		if (projectName.size() > 40)
		{
			QMessageBox::critical(this, qApp->applicationName(), tr("The project name is limited to 40 characters."));
			supposedProjectName = projectName;
			continue;
		}

		if (projectName.count(QRegularExpression("[A-Za-z_0-9]")) != projectName.size())
		{
			QMessageBox::critical(
				this,
				qApp->applicationName(),
				QString("The project name contains illegal characters: %1").arg(projectName.remove(QRegularExpression("[A-Za-z_0-9]"))));

			supposedProjectName = projectName;
			continue;
		}

		// Check if project with such name already exists
		//
		bool alreadyExists = std::any_of(projects.begin(),
										 projects.end(),
										 [&projectName](const DbProject& p) -> bool
										 {
											 return p.projectName().compare(projectName, Qt::CaseInsensitive) == 0;
										 });

		if (alreadyExists == true)
		{
			QMessageBox mb{this};
			mb.setIcon(QMessageBox::Icon::Critical);
			mb.setText(tr("Project %1 already exists.").arg(projectName));
			mb.setInformativeText(tr("Please, enter another project name."));
			mb.exec();
			supposedProjectName = projectName;
			continue;
		}

		// Ok
		//
		break;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QString error;
	bool ok = restorer.restore(projectName, fileName, server, error);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox mb{this};
		mb.setIcon(QMessageBox::Icon::Critical);
		mb.setText(tr("Restore failed."));
		mb.setInformativeText(error.left(511));
		mb.exec();
	}
	else
	{
		QMessageBox mb{QMessageBox::Icon::Information, qAppName(), tr("Restore completed successfully."), QMessageBox::Ok, this};
		mb.setInformativeText(tr("<b>NOTE! Check the project and restore it manually if it was not restored correctly.</b>"));
		mb.exec();
	}

	refreshProjectList();
	selectProject(projectName);
	return;
}

void ProjectsTabPage::closeProject()
{
	if (dbController()->isProjectOpened() == false)
	{
		assert(dbController()->isProjectOpened() == true);
		return;
	}

	assert(m_preCloseConditionsCallback);

	if (m_preCloseConditionsCallback() == true)
	{
		dbController()->closeProject(this);
	}

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
											 QString(),
											 &ok);

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
												   "cloned_" + projectName,
												   &ok);

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
											 QString(),
											 &ok);

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

		if (item != nullptr && item->text().toLower() == lcProjectName)
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
	menu.addAction(m_backupProjectAction);
	menu.addAction(m_restoreProjectAction);
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

	updateUiState(dbController()->isProjectOpened());

	return;
}
