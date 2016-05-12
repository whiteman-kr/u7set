#include "TuningMainWindow.h"
#include <QSettings>
#include <QFile>
#include <QGroupBox>
#include <QTableView>
#include <QMenuBar>


TuningMainWindow::TuningMainWindow(QString cfgPath, QWidget *parent) :
	QMainWindow(parent)//,
{
	QSettings settings("Radiy", "TuningIPEN");

	//UI
	//
	QRect desktopRect = QApplication::desktop()->screenGeometry(this);
	QPoint center = desktopRect.center();
	desktopRect.setSize(QSize(desktopRect.width() * 2 / 3, desktopRect.height() * 2 / 3));
	desktopRect.moveCenter(center);
	QRect windowRect = settings.value("TuningMainWindow/geometry", desktopRect).toRect();
	if (windowRect.height() > desktopRect.height())
	{
		windowRect.setHeight(desktopRect.height());
	}
	if (windowRect.width() > desktopRect.width())
	{
		windowRect.setWidth(desktopRect.width());
	}
	setGeometry(windowRect);

	setWindowTitle("Tuning IPEN");

	QTabWidget* tabs = new QTabWidget(this);
	setCentralWidget(tabs);

	QTabWidget* setOfSignalsScram = new QTabWidget;
	tabs->addTab(setOfSignalsScram, "Set of signals SCRAM");
	for (int i = 0; i < 3; i++)
	{
		setOfSignalsScram->addTab(new QTableView, QString("Safety channel %1").arg(i + 1));
	}

	tabs->addTab(new QWidget, "Automatic Power Regulator (APR)");

	menuBar()->addAction("Settings");

	statusBar();

	if (cfgPath == "")
	{
		m_cfgPath = settings.value("ConfigurationPath").toString();
	}
	else
	{
		m_cfgPath = cfgPath;
		settings.setValue("ConfigurationPath", m_cfgPath);
	}

	// run Tuning Service
	//
	TuningServiceWorker* worker = new TuningServiceWorker("Tuning Service", "", "", m_cfgPath + "/configuration.xml");

	m_service = new Service(worker);
	m_service->start();

	// wait while m_service started ?!
	//
	QVector<TuningDataSourceInfo> info;
	worker->getTuningDataSourcesInfo(info);
}


TuningMainWindow::~TuningMainWindow()
{
	m_service->stop();
	delete m_service;

	//delete ui;
}


