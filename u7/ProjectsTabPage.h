#ifndef DATABASETABPAGE_H
#define DATABASETABPAGE_H

#include "MainTabPage.h"

class QListWidget;
class DbStore;

class ProjectsTabPage : public MainTabPage
{
public:
	ProjectsTabPage(DbStore* dbstore, QWidget* parent);

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

public slots:
	void projectOpened();
	void projectClosed();

private slots:
	void createProject();
	void openProject();
	void closeProject();
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

#endif // DATABASETABPAGE_H
