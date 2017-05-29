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
	QTableWidget* m_pProjectTable = nullptr;

	QPushButton* m_pNewProject = nullptr;
	QPushButton* m_pOpenProject = nullptr;
	QPushButton* m_pCloseProject = nullptr;
	QPushButton* m_pDeleteProject = nullptr;
	QPushButton* m_pRefreshProjectList = nullptr;

	QAction* m_refreshAction = nullptr;
};

