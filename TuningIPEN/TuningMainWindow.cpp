#include "TuningMainWindow.h"
#include "ui_TuningMainWindow.h"
#include <QSettings>
#include <QFile>


TuningMainWindow::TuningMainWindow(QString cfgPath, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TuningMainWindow)
{
	ui->setupUi(this);

	QSettings settings("Radiy", "TuningIPEN");

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

	delete ui;
}


