#include "MainWindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include "Settings.h"
#include "ObjectFilter.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	if (theFilters.load("ObjectFilters.xml") == false)
	{
		QMessageBox::critical(nullptr, QString("Error"), theFilters.errorCode());
	}

	if (theUserFilters.load("ObjectFiltersUser.xml") == false)
	{
		QMessageBox::critical(nullptr, QString("Error"), theFilters.errorCode());
	}



	if (theSettings.m_mainWindowPos.x() != -1 && theSettings.m_mainWindowPos.y() != -1)
	{
		move(theSettings.m_mainWindowPos);
		restoreGeometry(theSettings.m_mainWindowGeometry);
		restoreState(theSettings.m_mainWindowState);
	}

	m_tuningWorkspace = new TuningWorkspace(this);

	setCentralWidget(m_tuningWorkspace);
}

MainWindow::~MainWindow()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();


	theFilters.save("ObjectFilters1.xml");
	theUserFilters.save("ObjectFiltersUser1.xml");
}
