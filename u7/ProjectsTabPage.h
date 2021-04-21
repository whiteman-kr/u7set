#pragma once

#include "MainTabPage.h"
#include "../DbLib/DbStruct.h"

class QListWidget;
class DbController;

class ProjectsTabPage : public MainTabPage
{
	Q_OBJECT
public:
	ProjectsTabPage(DbController* dbcontroller, QWidget* parent);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;

signals:
	void projectAboutToBeClosed();

public slots:
	void projectOpened(DbProject project);
	void projectClosed();

	private slots:
	void createProject();
	void openProject();
	void closeProject();
	void cloneProject();
	void deleteProject();
	void refreshProjectList();
	void selectProject(const QString &projectName);
	void projectsContextMenuRequested(const QPoint&pos);
	void projectsSortIndicatorChanged(int column, Qt::SortOrder order);
	void projectTableSelectionChanged();

	// Properties
	//
protected:

	// Data
	//
private:
	QTableWidget* m_projectTable = nullptr;

	QPushButton* m_newProjectButton = nullptr;
	QPushButton* m_openProjectButton = nullptr;
	QPushButton* m_closeProjectButton = nullptr;
	QPushButton* m_cloneProjectButton = nullptr;
	QPushButton* m_deleteProjectButton = nullptr;
	QPushButton* m_refreshProjectListButton = nullptr;

	QAction* m_newProjectAction = nullptr;
	QAction* m_openProjectAction = nullptr;
	QAction* m_closeProjectAction = nullptr;
	QAction* m_cloneProjectAction = nullptr;
	QAction* m_deleteProjectAction = nullptr;
	QAction* m_refreshAction = nullptr;
};

