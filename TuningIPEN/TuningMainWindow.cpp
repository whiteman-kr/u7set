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
//#include <QScrollBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>


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


Signal* findSignal(const QString& id, QVector<Tuning::TuningDataSourceInfo>& sourceInfoVector)
{
	for (auto& sourceInfo : sourceInfoVector)
	{
		for (auto& signal : sourceInfo.tuningSignals)
		{
			if (signal.appSignalID() == id)
			{
				return &signal;
			}
		}
	}
	return nullptr;
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
	Tuning::TuningIPENServiceWorker* worker = new Tuning::TuningIPENServiceWorker("Tuning Service", "", "", m_cfgPath + "/configuration.xml");

	m_service = new Tuning::TuningService(worker);

	connect(m_service, &Tuning::TuningService::tuningServiceReady, this, &TuningMainWindow::onTuningServiceReady);
	connect(m_service, &Tuning::TuningService::userRequest, this, &TuningMainWindow::onUserRequest);
	connect(m_service, &Tuning::TuningService::replyWithNoZeroFlags, this, &TuningMainWindow::onReplyWithNoZeroFlags);

	m_service->start();

	connect(m_updateTimer, &QTimer::timeout, this, &TuningMainWindow::updateSignalStates);
	m_updateTimer->start(200);

	//Init tabs
	//
	QTabWidget* tabs = new QTabWidget(this);
	setCentralWidget(tabs);

	m_automaticPowerRegulatorWidget = new QWidget;

	tabs->addTab(m_automaticPowerRegulatorWidget, "Automatic Power Regulator (APR)");

	m_setOfSignalsScram = new QTabWidget(this);
	QWidget* widget = new QWidget;
	QHBoxLayout* hl = new QHBoxLayout;
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


void TuningMainWindow::addAnalogSetter(QFormLayout* fl, QVector<Tuning::TuningDataSourceInfo>& sourceInfoVector, QString label, QString id, double highLimit)
{
	double lowLimit = 0;

	Signal* signal = findSignal(id, sourceInfoVector);
	label = label + "\n" + id;
	if (signal != nullptr)
	{
		label = signal->caption().trimmed() + "\n" + signal->customAppSignalID().trimmed();
		lowLimit = signal->lowEngeneeringUnits();
		highLimit = signal->highEngeneeringUnits();
	}

	auto setter = new AnalogSignalSetter(id, lowLimit, highLimit, m_service, this);
	fl->addRow(label, setter);

	connect(m_updateTimer, &QTimer::timeout, setter, &AnalogSignalSetter::updateValue);
	connect(m_service, &Tuning::TuningService::signalStateReady, setter, &AnalogSignalSetter::setCurrentValue);
}


template<typename T>
void writeField(QTextStream& out, QString caption, T field)
{
	caption += ':';
	while (caption.length() < 20)
	{
		caption += ' ';
	}
	out << QString("%1 0x%2 = 0b%3 = %4").arg(caption).arg(field, sizeof(field) * 2, 16, QChar('0')).arg(field, sizeof(field) * 8, 2, QChar('0')).arg(field) << endl;
}


void writeBuffer(QTextStream& out, QString caption, quint8* buffer, int size)
{
	out << caption << " (hex)";
	for (int i = 0; i < size; i++)
	{
		if (i % 16 == 0)
		{
			out << endl << QString("%1: ").arg(i, 3, 16, QChar('0'));
		}
		out << QString("%1 ").arg(buffer[i], sizeof(*buffer) * 2, 16, QChar('0'));
	}
	out << endl;
}


void TuningMainWindow::writeFrameToLog(QString caption, FotipFrame& fotipFrame)
{
	QFile file("TuningIPEN.log");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
	{
		return;
	}

	QTextStream out(&file);

	out << QString("------------------------------ Frame info of %1").arg(caption) << endl;
	out << QString("At ") << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz") << endl;
	out << QString("-------------------- Header (hex, bin, dec)") << endl;
	auto& header = fotipFrame.header;

	writeField(out, "Protocol version", header.protocolVersion);
	writeField(out, "Unique ID", header.uniqueId);
	writeField(out, "Subsystem key", header.subsystemKeyWord);
	writeField(out, "Operation code", header.operationCode);
	writeField(out, "Flags", header.flags.all);
	writeField(out, "Start address", header.startAddress);
	writeField(out, "Fotip frame size", header.fotipFrameSize);
	writeField(out, "Rom size", header.romSize);
	writeField(out, "Rom frame size", header.romFrameSize);
	writeField(out, "Data type", header.dataType);

	writeBuffer(out, "---------- Header reserve", header.reserve, FOTIP_HEADER_RESERVE_SIZE);

	writeBuffer(out, "------------------------------ Data", reinterpret_cast<quint8*>(fotipFrame.data), FOTIP_TX_RX_DATA_SIZE);
	writeBuffer(out, "------------------------------ Comparison result", reinterpret_cast<quint8*>(fotipFrame.comparisonResult), FOTIP_COMPARISON_RESULT_SIZE);
	writeBuffer(out, "------------------------------ Data reserv", reinterpret_cast<quint8*>(fotipFrame.reserv), FOTIP_DATA_RESERV_SIZE);

	out << endl << endl;

	file.close();
}


void TuningMainWindow::updateSignalStates()
{
	m_service->getSignalState("#HP01LC01DC_01PPC");
	m_service->getSignalState("#HP01LC02RAM_01PPC");
}


void TuningMainWindow::updateSignalState(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid)
{
	Q_UNUSED(lowLimit);
	Q_UNUSED(highLimit);
	/*if (appSignalID == "#HP01LC01DC_01PPC")
	{
		m_scrollBar->setValue(value * 10);
	}*/
	if (appSignalID == "#HP01LC02RAM_01PPC")
	{
		m_automaticMode->setChecked(currentValue != 0 && valid != 0);
	}
}

void TuningMainWindow::updateDataSourceStatus(Tuning::TuningDataSourceState state)
{
	if (m_statusLabelMap.contains(state.lmEquipmentID))
	{
		QLabel* label = m_statusLabelMap.value(state.lmEquipmentID, nullptr);
		assert(label != nullptr);

		if (!state.hasConnection)
		{
			label->setStyleSheet("QLabel { background-color : #FF3333; }");	//red
		}
		else
		{
			if (state.flags.all != 0)
			{
				label->setStyleSheet("QLabel { background-color : #FFCC33; }");	//yellow
			}
			else
			{
				label->setStyleSheet("QLabel { background-color : #009933; }");	//green
			}
		}
	}
}


/*void TuningMainWindow::applyNewScrollBarValue()
{
	double newValue = m_scrollBar->value() / 10.0;

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>#HP01LC01DC_01PPC</b> signal value to <b>%1</b>?")
									   .arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState("#HP01LC02RAM_01PPC", newValue);
}*/


void TuningMainWindow::applyNewAutomaticMode(bool enabled)
{
	m_automaticMode->setChecked(!enabled);

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>HP01LC02RAM_01PPC</b> signal value to <b>%1</b>?")
									   .arg(enabled ? "Yes" : "No"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState("#HP01LC02RAM_01PPC", enabled ? 1 : 0);
}


void TuningMainWindow::onTuningServiceReady()
{
	m_service->getTuningDataSourcesInfo(m_info);

	QVector<int> sourceIndexes;

	for (int index = 0; index < m_info.count(); index++)
	{
		Tuning::TuningDataSourceInfo& sourceInfo = m_info[index];
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
		sourceIndexes.insert(place, index);

		SafetyChannelSignalsModel* model = new SafetyChannelSignalsModel(sourceInfo, m_service, this);
		view->setModel(model);
		SafetyChannelSignalsDelegate* delegate = new SafetyChannelSignalsDelegate;
		connect(delegate, &SafetyChannelSignalsDelegate::aboutToChangeDiscreteSignal, model, &SafetyChannelSignalsModel::changeDiscreteSignal);
		view->setItemDelegate(delegate);

		connect(m_service, &Tuning::TuningService::signalStateReady, model, &SafetyChannelSignalsModel::updateSignalState);

		QFont font = view->font();
		font.setPointSize(font.pointSize() * 1.2);
		view->setFont(font);

		view->resizeColumnsToContents();
	}

	for (int i = 0; i < sourceIndexes.count(); i++)
	{
		Tuning::TuningDataSourceInfo& sourceInfo = m_info[sourceIndexes[i]];
		QLabel* newLabel = new QLabel(sourceInfo.lmCaption + ": " + sourceInfo.lmAddressPort.addressStr(), this);
		statusBar()->addWidget(newLabel);

		m_statusLabelMap.insert(sourceInfo.lmEquipmentID, newLabel);
	}

	connect(m_service, &Tuning::TuningService::tuningDataSourceStateUpdate, this, &TuningMainWindow::updateDataSourceStatus);
	connect(m_service, &Tuning::TuningService::signalStateReady, this, &TuningMainWindow::updateSignalState);

	QHBoxLayout* hl = new QHBoxLayout;
	m_automaticPowerRegulatorWidget->setLayout(hl);

	QGroupBox* groupBox = new QGroupBox("Power control", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	QFormLayout* fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	groupBox->setLayout(fl);

	addAnalogSetter(fl, m_info, "Power demand control", "#HP01LC01DC_01PPC", 110);

	/*m_scrollBar = new QScrollBar(Qt::Horizontal, this);
	m_scrollBar->setMinimum(0);
	m_scrollBar->setMaximum(1100);
	m_scrollBar->setPageStep(1);
	m_scrollBar->setTracking(false);
	connect(m_scrollBar, &QScrollBar::sliderMoved, this, &TuningMainWindow::applyNewScrollBarValue);

	fl->addRow("#HP01LC01DC_01PPC", m_scrollBar);*/

	m_automaticMode = new QPushButton("Automatic mode", this);
	m_automaticMode->setCheckable(true);
	connect(m_automaticMode, &QPushButton::clicked, this, &TuningMainWindow::applyNewAutomaticMode);

	QString automaticModeId = "#HP01LC02RAM_01PPC";
	Signal* signal = findSignal(automaticModeId, m_info);
	if (signal == nullptr)
	{
		fl->addRow(automaticModeId, m_automaticMode);
	}
	else
	{
		fl->addRow(signal->caption().trimmed() + "\n" + signal->customAppSignalID().trimmed(), m_automaticMode);
	}
	hl->addWidget(groupBox);

	groupBox = new QGroupBox("Setting coeficients", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	groupBox->setLayout(fl);
	addAnalogSetter(fl, m_info, "Limiter range command \"UP\"", "#HP01LC01RLR_01PPC", 100);
	addAnalogSetter(fl, m_info, "Limiter range command \"DOWN\"", "#HP01LC01RLR_02PPC", 100);
	addAnalogSetter(fl, m_info, "Scaling coefficient", "#HP01LC02RCC_01PPC", 100);
	addAnalogSetter(fl, m_info, "Driving coefficient", "#HP01LC02RDC_01PPC", 100);
	hl->addWidget(groupBox);

	QFont font = m_automaticPowerRegulatorWidget->font();
	font.setPointSize(font.pointSize() * 1.4);
	setFontRecursive(m_automaticPowerRegulatorWidget, font);
}


void TuningMainWindow::onUserRequest(FotipFrame fotipFrame)
{
	writeFrameToLog("User request", fotipFrame);
}


void TuningMainWindow::onReplyWithNoZeroFlags(FotipFrame fotipFrame)
{
	writeFrameToLog("Reply with no zero flags", fotipFrame);
}
