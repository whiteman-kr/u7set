#include "TuningMainWindow.h"
#include "SafetyChannelSignalsModel.h"
#include "AnalogSignalSetter.h"
#include <QSettings>
#include <QFile>
#include <QGroupBox>
#include <QTableView>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QMessageBox>


void setFontRecursive(QWidget* parentWidget, const QFont& font)
{
	parentWidget->setFont(font);
	for (auto object : parentWidget->children())
	{
		QWidget* childWidget = qobject_cast<QWidget*>(object);
		if (childWidget != nullptr)
		{
			setFontRecursive(childWidget, font);
		}
	}
}


TuningMainWindow::TuningMainWindow(QString cfgPath, QWidget *parent) :
	QMainWindow(parent),
	m_updateTimer(new QTimer(this))
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

	connect(m_updateTimer, &QTimer::timeout, this, &TuningMainWindow::updateSignalStates);
	m_updateTimer->start(200);

	//Init tabs
	//
	QTabWidget* tabs = new QTabWidget(this);
	setCentralWidget(tabs);

	QWidget* widget = new QWidget;
	QHBoxLayout* hl = new QHBoxLayout;
	widget->setLayout(hl);

	QGroupBox* groupBox = new QGroupBox("Power control", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	QFormLayout* fl = new QFormLayout;
	groupBox->setLayout(fl);
	addAnalogSetter(fl, "Power demand control", "#HP01LC01A_01MAT", 110);

	m_scrollBar = new QScrollBar(Qt::Horizontal, this);
	m_scrollBar->setMinimum(0);
	m_scrollBar->setMaximum(1100);
	m_scrollBar->setPageStep(1);
	m_scrollBar->setTracking(false);
	connect(m_scrollBar, &QScrollBar::sliderMoved, this, &TuningMainWindow::applyNewScrollBarValue);

	fl->addRow("#HP01LC01DC_01PPC", m_scrollBar);

	m_automaticMode = new QPushButton("Automatic mode", this);
	m_automaticMode->setCheckable(true);
	connect(m_automaticMode, &QPushButton::toggled, this, &TuningMainWindow::applyNewAutomaticMode);

	fl->addRow("#HP01LC02RAM_01PPC", m_automaticMode);
	hl->addWidget(groupBox);

	groupBox = new QGroupBox("Setting coeficients", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	fl = new QFormLayout;
	groupBox->setLayout(fl);
	addAnalogSetter(fl, "Limiter range command \"UP\"", "#HP01LC01RLR_01PPC", 100);
	addAnalogSetter(fl, "Limiter range command \"DOWN\"", "#HP01LC01RLR_02PPC", 100);
	addAnalogSetter(fl, "Scaling coefficient", "#HP01LC02RCC_01PPC", 100);
	addAnalogSetter(fl, "Driving coefficient", "#HP01LC02RDC_01PPC", 100);
	hl->addWidget(groupBox);

	tabs->addTab(widget, "Automatic Power Regulator (APR)");

	QFont font = widget->font();
	font.setPointSize(font.pointSize() * 1.4);
	setFontRecursive(widget, font);

	m_setOfSignalsScram = new QTabWidget(this);
	widget = new QWidget;
	hl = new QHBoxLayout;
	widget->setLayout(hl);
	hl->addWidget(m_setOfSignalsScram);
	tabs->addTab(widget, "Set of signals SCRAM");
}


TuningMainWindow::~TuningMainWindow()
{
	m_updateTimer->stop();
	m_service->stop();
	delete m_service;
}


void TuningMainWindow::addAnalogSetter(QFormLayout* fl, QString label, QString id, double highLimit)
{
	auto setter = new AnalogSignalSetter(id, highLimit, m_service, this);
	fl->addRow(label + "\n" + id, setter);
	connect(m_updateTimer, &QTimer::timeout, setter, &AnalogSignalSetter::updateValue);
	connect(m_service, &TuningService::signalStateReady, setter, &AnalogSignalSetter::setCurrentValue);
}


void TuningMainWindow::updateSignalStates()
{
	m_service->getSignalState("#HP01LC01DC_01PPC");
	m_service->getSignalState("#HP01LC02RAM_01PPC");
}


void TuningMainWindow::updateSignalState(QString appSignalID, double value)
{
	if (appSignalID == "#HP01LC01DC_01PPC")
	{
		m_scrollBar->setValue(value * 10);
	}
	if (appSignalID == "#HP01LC02RAM_01PPC")
	{
		m_automaticMode->setChecked(value != 0);
	}
}


void TuningMainWindow::applyNewScrollBarValue()
{
	double newValue = m_scrollBar->value() / 10.0;

	/*auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>#HP01LC01DC_01PPC</b> signal value to <b>%1</b>?")
									   .arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}*/

	m_service->setSignalState("#HP01LC02RAM_01PPC", newValue);
}


void TuningMainWindow::applyNewAutomaticMode(bool enabled)
{
	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>#HP01LC02RAM_01PPC</b> signal value to <b>%1</b>?")
									   .arg(enabled ? 1 : 0), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState("#HP01LC02RAM_01PPC", enabled ? 1 : 0);
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
