#include "TrendsMainWindow.h"
#include "ui_TrendsMainWindow.h"
#include "Settings.h"
#include "TrendWidget.h"

TrendsMainWindow::TrendsMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TrendsMainWindow)
{
	ui->setupUi(this);

	setMinimumSize(500, 300);
	restoreWindowState();

	QGridLayout* layout = new QGridLayout;

	centralWidget()->setLayout(layout);

	TrendLib::TrendWidget* trendWidget = new TrendLib::TrendWidget;
	layout->addWidget(trendWidget, 0, 0);

	connect(ui->actionOpen, &QAction::triggered, this, &TrendsMainWindow::actionOpenTriggered);
	connect(ui->actionSave, &QAction::triggered, this, &TrendsMainWindow::actionSaveTriggered);
	connect(ui->actionExit, &QAction::triggered, this, &TrendsMainWindow::actionExitTriggered);
	connect(ui->actionAbout, &QAction::triggered, this, &TrendsMainWindow::actionAboutTriggered);

	return;
}

TrendsMainWindow::~TrendsMainWindow()
{
	delete ui;
}

void TrendsMainWindow::saveWindowState()
{
	theSettings.m_mainWindowPos = pos();
	theSettings.m_mainWindowGeometry = saveGeometry();
	theSettings.m_mainWindowState = saveState();

	theSettings.writeUserScope();

	return;
}

void TrendsMainWindow::restoreWindowState()
{
	move(theSettings.m_mainWindowPos);
	restoreGeometry(theSettings.m_mainWindowGeometry);
	restoreState(theSettings.m_mainWindowState);

	// Ensure widget is visible
	//
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	QRect screenRect  = QApplication::desktop()->availableGeometry(this);
	QRect intersectRect = screenRect.intersected(frameGeometry());

	if (isMaximized() == false &&
		(intersectRect.width() < size().width() ||
		 intersectRect.height() < size().height()))
	{
		move(screenRect.topLeft());
	}

	if (isMaximized() == false &&
		(frameGeometry().width() > screenRect.width() ||
		 frameGeometry().height() > screenRect.height()))
	{
		resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
	}

	return;
}

void TrendsMainWindow::closeEvent(QCloseEvent* e)
{
	saveWindowState();
	e->accept();
	return;
}

void TrendsMainWindow::timerEvent(QTimerEvent*)
{

}

void TrendsMainWindow::showEvent(QShowEvent*)
{

}

void TrendsMainWindow::actionOpenTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionSaveTriggered()
{
	// todo
	//
	assert(false);
}

void TrendsMainWindow::actionExitTriggered()
{
	close();
}

void TrendsMainWindow::actionAboutTriggered()
{
	// todo
	//
	assert(false);
}

