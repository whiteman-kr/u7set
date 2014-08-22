#pragma once

#include "MainTabPage.h"

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
	void projectOpened();
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
	QTableWidget* m_pProjectTable;

	QPushButton* m_pNewProject;
	QPushButton* m_pOpenProject;
	QPushButton* m_pCloseProject;
	QPushButton* m_pDeleteProject;
	QPushButton* m_pRefreshProjectList;
};

