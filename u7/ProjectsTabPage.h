#pragma once

#include "MainTabPage.h"
#include "../lib/DbStruct.h"

class QListWidget;
class DbController;

class ProjectsTabPage : public MainTabPage
{
public:
	ProjectsTabPage(DbController* dbcontroller, QWidget* parent);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;

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

	QAction* m_refreshAction = nullptr;
};

