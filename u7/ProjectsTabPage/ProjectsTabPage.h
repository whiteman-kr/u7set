#pragma once

#include "../MainTabPage.h"

class ProjectsTabPage : public MainTabPage
{
	Q_OBJECT
public:
	ProjectsTabPage(DbController* dbcontroller, std::function<bool(void)> preCloseConditionsCallback, QWidget* parent);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

public slots:
	void projectOpened(DbProject project);
	void projectClosed();

	void refreshProjectList();

private slots:
	void updateUiState(bool isOpened);

	void createProject();
	void openProject();
	void backupProject();
	void restoreProject();
	void closeProject();
	void cloneProject();
	void deleteProject();
	void selectProject(const QString& projectName);
	void projectsContextMenuRequested(const QPoint& pos);
	void projectsSortIndicatorChanged(int column, Qt::SortOrder order);
	void projectTableSelectionChanged();

	// Data
	//
private:
	QTableWidget* m_projectTable = nullptr;

	QPushButton* m_newProjectButton = nullptr;
	QPushButton* m_openProjectButton = nullptr;
	QPushButton* m_closeProjectButton = nullptr;

	QPushButton* m_backupProjectButton = nullptr;
	QPushButton* m_restoreProjectButton = nullptr;

	QPushButton* m_cloneProjectButton = nullptr;
	QPushButton* m_deleteProjectButton = nullptr;

	QPushButton* m_refreshProjectListButton = nullptr;

	QAction* m_newProjectAction = nullptr;
	QAction* m_openProjectAction = nullptr;
	QAction* m_closeProjectAction = nullptr;

	QAction* m_backupProjectAction = nullptr;
	QAction* m_restoreProjectAction = nullptr;

	QAction* m_cloneProjectAction = nullptr;
	QAction* m_deleteProjectAction = nullptr;

	QAction* m_refreshAction = nullptr;

	std::function<bool(void)> m_preCloseConditionsCallback; // if returns true, then project can be closed,
															// if returns false, then closing project must be stopped
};
