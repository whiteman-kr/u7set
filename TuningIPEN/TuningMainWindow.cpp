#include "TuningMainWindow.h"
#include "SafetyChannelSignalsModel.h"
#include <QSettings>
#include <QFile>
#include <QGroupBox>
#include <QTableView>
#include <QMenuBar>
#include <QHBoxLayout>


TuningMainWindow::TuningMainWindow(QString cfgPath, QWidget *parent) :
	QMainWindow(parent)//,
{
	QSettings settings("Radiy", "TuningIPEN");

	//Window geometry
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

	/*QWidget* widget = new QWidget;
	QHBoxLayout* hl = new QHBoxLayout;
	widget->setLayout(hl);
	hl->addWidget(new QGroupBox(this));

	hl->addWidget(new QGroupBox(this));*/
	tabs->addTab(new QWidget, "Automatic Power Regulator (APR)");

	m_setOfSignalsScram = new QTabWidget(this);
	QWidget* widget = new QWidget;
	QHBoxLayout* hl = new QHBoxLayout;
	widget->setLayout(hl);
	hl->addWidget(m_setOfSignalsScram);
	tabs->addTab(widget, "Set of signals SCRAM");

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

	m_service = new TuningService(worker);

	connect(m_service, &TuningService::tuningServiceReady, this, &TuningMainWindow::onTuningServiceReady);

	m_service->start();
}


TuningMainWindow::~TuningMainWindow()
{
	m_service->stop();
	delete m_service;
}


void TuningMainWindow::onTuningServiceReady()
{
	m_service->getTuningDataSourcesInfo(m_info);

	for (TuningDataSourceInfo& sourceInfo : m_info)
	{
		int place = 0;
		for (; place < m_setOfSignalsScram->count(); place++)
		{
			if (sourceInfo.lmCaption < m_setOfSignalsScram->tabText(place))
			{
				break;
			}
		}
		QTableView* view = new QTableView;
		m_setOfSignalsScram->insertTab(place, view, sourceInfo.lmCaption);

		SafetyChannelSignalsModel* model = new SafetyChannelSignalsModel(sourceInfo, m_service, this);
		view->setModel(model);

		connect(m_service, &TuningService::signalStateReady, model, &SafetyChannelSignalsModel::updateSignalState, Qt::QueuedConnection);

		view->resizeColumnsToContents();
	}
}
