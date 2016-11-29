#include "TuningMainWindow.h"
#include "SafetyChannelSignalsModel.h"
#include "TripleChannelSignalsModel.h"
#include "AnalogSignalSetter.h"
#include "DiscreteSignalSetter.h"
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
#include <QStatusBar>
#include <QLabel>
#include <QPixmap>


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


Signal* findSignal(const QString& id, QVector<TuningIPEN::TuningSourceInfo>& sourceInfoVector)
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


TuningMainWindow::TuningMainWindow(QString buildPath, QWidget *parent) :
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

	//menuBar()->addAction("Settings");

	statusBar();

	if (buildPath == "")
	{
		m_cfgPath = settings.value("ConfigurationPath").toString();
	}
	else
	{
		m_cfgPath = buildPath;
		settings.setValue("ConfigurationPath", m_cfgPath);
	}

	// run Tuning Service
	//
	TuningIPEN::TuningIPENServiceWorker* worker = new TuningIPEN::TuningIPENServiceWorker("Tuning Service", buildPath);

	m_service = new TuningIPEN::TuningIPENService(worker);

	connect(m_service, &TuningIPEN::TuningIPENService::tuningServiceReady, this, &TuningMainWindow::onTuningServiceReady);

	m_logWriter = new LogWriter;
	m_logThread = new QThread(this);
	m_logWriter->moveToThread(m_logThread);
	connect(m_service, &TuningIPEN::TuningIPENService::userRequest, m_logWriter, &LogWriter::onUserRequest, Qt::QueuedConnection);
	connect(m_service, &TuningIPEN::TuningIPENService::replyWithNoZeroFlags, m_logWriter, &LogWriter::onReplyWithNoZeroFlags, Qt::QueuedConnection);

	m_service->start();

	connect(m_updateTimer, &QTimer::timeout, this, &TuningMainWindow::updateSignalStates);
	m_updateTimer->start(200);

	//Init tabs
	//
	QTabWidget* mainTabs = new QTabWidget(this);

	QLabel* image = new QLabel(this);
	image->setPixmap(QPixmap(":/Images/u7/Images/logo.png"));

	QLabel* title = new QLabel("Safety Systems Control Consol for IEA-R1", this);
	QFont font = title->font();
	font.setPointSize(font.pointSize() * 2);
	title->setFont(font);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(image);
	hl->addStretch();
	hl->addWidget(title);

	QVBoxLayout* vl = new QVBoxLayout;
	vl->addLayout(hl);
	vl->addWidget(mainTabs);

	QWidget* container = new QWidget(this);
	container->setLayout(vl);
	setCentralWidget(container);

	// ========== First tab ==========
	m_setOfSignalsScram = new QTabWidget(this);
	QWidget* widget = new QWidget;
	hl = new QHBoxLayout;
	widget->setLayout(hl);
	hl->addWidget(m_setOfSignalsScram);
	mainTabs->addTab(widget, "Set of signals SCRAM");
	// ========== First tab ==========

	// ========== Second tab ==========
	m_beamDoorsWidget = new QTableView;
	mainTabs->addTab(m_beamDoorsWidget, "Deactivate signals \"Open beam doors\"");
	// ========== Second tab ==========

	// ========== Third tab ==========
	m_reactivityWidget = new QWidget;
	mainTabs->addTab(m_reactivityWidget, "Set of signals Canais Nucleare. Reactivity");
	// ========== Third tab ==========

	// ========== Fourth tab ==========
	m_automaticPowerRegulatorWidget = new QWidget;
	mainTabs->addTab(m_automaticPowerRegulatorWidget, "Automatic Power Regulator (APR)");
	// ========== Fourth tab ==========
}


TuningMainWindow::~TuningMainWindow()
{
	m_logThread->quit();
	m_logThread->wait();
	delete m_logThread;
	delete m_logWriter;

	m_updateTimer->stop();
	m_service->stop();
	delete m_service;
}


AnalogSignalSetter* TuningMainWindow::addAnalogSetter(QFormLayout* fl, QVector<TuningIPEN::TuningSourceInfo>& sourceInfoVector, QString label, QString id, double highLimit)
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
	connect(m_service, &TuningIPEN::TuningIPENService::signalStateReady, setter, &AnalogSignalSetter::setCurrentValue);

	return setter;
}

DiscreteSignalSetter* TuningMainWindow::addDiscreteSetter(QFormLayout* fl, QVector<TuningIPEN::TuningSourceInfo>& sourceInfoVector, QString label, QString id)
{
	auto setter = new DiscreteSignalSetter(id, label, m_service, this);

	Signal* signal = findSignal(id, sourceInfoVector);
	if (signal == nullptr)
	{
		fl->addRow(id, setter);
	}
	else
	{
		fl->addRow(signal->caption().trimmed() + "\n" + signal->customAppSignalID().trimmed(), setter);
	}

	connect(m_updateTimer, &QTimer::timeout, setter, &DiscreteSignalSetter::updateValue);
	connect(m_service, &TuningIPEN::TuningIPENService::signalStateReady, setter, &DiscreteSignalSetter::setCurrentValue);

	return setter;
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


void TuningMainWindow::updateSignalStates()
{
	m_service->getSignalState("#HP01LC02RAM_01PPC");
}


void TuningMainWindow::updateSignalState(QString /*appSignalID*/, double /*currentValue*/, double /*lowLimit*/, double /*highLimit*/, bool /*valid*/)
{
}

void TuningMainWindow::updateDataSourceStatus(TuningIPEN::TuningSourceState state)
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


void TuningMainWindow::applyNewScrollBarValue()
{
	double newValue = m_scrollBar->value() * 10;
	if (newValue == 0)
	{
		newValue = 1;
	}
	emit scrollBarMoved(newValue);
}


void TuningMainWindow::onTuningServiceReady()
{
	m_service->getTuningDataSourcesInfo(m_info);

	QVector<int> sourceIndexes;


	TripleChannelSignalsModel* tripleChannelSignalsModel = new TripleChannelSignalsModel(m_info, m_service, m_beamDoorsWidget);
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#YA10F04A_01PPC" << "#YA20F04A_01PPC" << "#YA30F04A_01PPC");

	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G01_01PPC" << "#TR02G01_01PPC" << "#TR03G01_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G02_01PPC" << "#TR02G02_01PPC" << "#TR03G02_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G03_01PPC" << "#TR02G03_01PPC" << "#TR03G03_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G04_01PPC" << "#TR02G04_01PPC" << "#TR03G04_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G05_01PPC" << "#TR02G05_01PPC" << "#TR03G05_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G06_01PPC" << "#TR02G06_01PPC" << "#TR03G06_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G07_01PPC" << "#TR02G07_01PPC" << "#TR03G07_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G08_01PPC" << "#TR02G08_01PPC" << "#TR03G08_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G09_01PPC" << "#TR02G09_01PPC" << "#TR03G09_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G10_01PPC" << "#TR02G10_01PPC" << "#TR03G10_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G11_01PPC" << "#TR02G11_01PPC" << "#TR03G11_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G12_01PPC" << "#TR02G12_01PPC" << "#TR03G12_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G13_01PPC" << "#TR02G13_01PPC" << "#TR03G13_01PPC");
	tripleChannelSignalsModel->addTripleSignal(QVector<QString>() << "#TR01G14_01PPC" << "#TR02G14_01PPC" << "#TR03G14_01PPC");

	// ========== First tab ==========
	for (int index = 0; index < m_info.count(); index++)
	{
		TuningIPEN::TuningSourceInfo& sourceInfo = m_info[index];
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

		SafetyChannelSignalsModel* model = new SafetyChannelSignalsModel(sourceInfo, m_service, view);
		SafetyChannelSignalsProxyModel* proxyModel = new SafetyChannelSignalsProxyModel(tripleChannelSignalsModel, model, view);
		proxyModel->setSourceModel(model);
		view->setModel(proxyModel);
		SafetyChannelSignalsDelegate* delegate = new SafetyChannelSignalsDelegate(view);
		connect(delegate, &SafetyChannelSignalsDelegate::aboutToChangeDiscreteSignal, model, &SafetyChannelSignalsModel::changeDiscreteSignal);
		view->setItemDelegate(delegate);

		QFont font = view->font();
		font.setPointSize(font.pointSize() * 1.2);
		view->setFont(font);

		view->resizeColumnsToContents();
	}
	// ========== First tab ==========

	// ========== Status bar ==========
	for (int i = 0; i < sourceIndexes.count(); i++)
	{
		TuningIPEN::TuningSourceInfo& sourceInfo = m_info[sourceIndexes[i]];
		QLabel* newLabel = new QLabel(sourceInfo.lmCaption + ": " + sourceInfo.lmAddressPort.addressStr(), this);
		statusBar()->addWidget(newLabel);

		m_statusLabelMap.insert(sourceInfo.lmEquipmentID, newLabel);
		}
	// ========== Status bar ==========

	connect(m_service, &TuningIPEN::TuningIPENService::tuningDataSourceStateUpdate, this, &TuningMainWindow::updateDataSourceStatus);
	connect(m_service, &TuningIPEN::TuningIPENService::signalStateReady, this, &TuningMainWindow::updateSignalState);

	// ========== Second tab ==========
	QFont font = m_beamDoorsWidget->font();
	font.setPointSize(font.pointSize() * 1.3);
	m_beamDoorsWidget->setFont(font);

	m_beamDoorsWidget->setModel(tripleChannelSignalsModel);
	m_beamDoorsWidget->setWordWrap(false);
	m_beamDoorsWidget->resizeRowsToContents();
	m_beamDoorsWidget->resizeColumnsToContents();

	TripleChannelSignalsDelegate* delegate = new TripleChannelSignalsDelegate(m_beamDoorsWidget);
	connect(delegate, &TripleChannelSignalsDelegate::aboutToChangeDiscreteSignal, tripleChannelSignalsModel, &TripleChannelSignalsModel::changeDiscreteSignal);
	m_beamDoorsWidget->setItemDelegate(delegate);
	// ========== Second tab ==========

	// ========== Third tab ==========
	QVBoxLayout* vl = new QVBoxLayout;
	m_reactivityWidget->setLayout(vl);

	QFormLayout* fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	fl->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	vl->addLayout(fl);
	addAnalogSetter(fl, m_info, "The average lifetime of prompt neutrons", "#HP01SC03A_01PPC", 0.001);

	QHBoxLayout* hl = new QHBoxLayout;

	fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	hl->addLayout(fl);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 1st group", "#HP01SC03A_02PPC", 0.3);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 2st group", "#HP01SC03A_04PPC", 0.3);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 3st group", "#HP01SC03A_06PPC", 0.3);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 4st group", "#HP01SC03A_08PPC", 0.3);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 5st group", "#HP01SC03A_10PPC", 0.3);
	addAnalogSetter(fl, m_info, "Proportion of delayed neutrons of the 6st group", "#HP01SC03A_12PPC", 0.3);

	fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	hl->addLayout(fl);
	addAnalogSetter(fl, m_info, "Constant decay time of the 1st group", "#HP01SC03A_03PPC", 5);
	addAnalogSetter(fl, m_info, "Constant decay time of the 2st group", "#HP01SC03A_05PPC", 5);
	addAnalogSetter(fl, m_info, "Constant decay time of the 3st group", "#HP01SC03A_07PPC", 5);
	addAnalogSetter(fl, m_info, "Constant decay time of the 4st group", "#HP01SC03A_09PPC", 5);
	addAnalogSetter(fl, m_info, "Constant decay time of the 5st group", "#HP01SC03A_11PPC", 5);
	addAnalogSetter(fl, m_info, "Constant decay time of the 6st group", "#HP01SC03A_13PPC", 5);

	vl->addLayout(hl);

	fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	fl->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	vl->addLayout(fl);
	addAnalogSetter(fl, m_info, "The total fraction of delayed neutrons", "#HP01SC03A_14PPC", 1);

	vl->addStretch();

	font = m_reactivityWidget->font();
	font.setPointSize(font.pointSize() * 1.4);
	setFontRecursive(m_reactivityWidget, font);
	// ========== Third tab ==========

	// ========== Fourth tab ==========
	hl = new QHBoxLayout;
	m_automaticPowerRegulatorWidget->setLayout(hl);

	QGroupBox* groupBox = new QGroupBox("Power control", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	fl = new QFormLayout;
	fl->setVerticalSpacing(20);

	vl = new QVBoxLayout;
	vl->addLayout(fl);
	groupBox->setLayout(vl);

	auto powerDemandControlSetter = addAnalogSetter(fl, m_info, "Power demand control", "#HP01LC01DC_01PPC", 110);
	connect(this, &TuningMainWindow::scrollBarMoved, powerDemandControlSetter, &AnalogSignalSetter::changeNewValue);
	connect(this, &TuningMainWindow::automaticModeChanged, powerDemandControlSetter, &AnalogSignalSetter::setDisabled);

	m_scrollBar = new QScrollBar(Qt::Horizontal, this);
	m_scrollBar->setMinimum(0);
	m_scrollBar->setMaximum(10);
	m_scrollBar->setPageStep(1);
	m_scrollBar->setTracking(false);
	connect(m_scrollBar, &QScrollBar::valueChanged, this, &TuningMainWindow::applyNewScrollBarValue);
	connect(this, &TuningMainWindow::automaticModeChanged, m_scrollBar, &QScrollBar::setDisabled);

	fl->addRow("", m_scrollBar);

	auto automaticModeSetter = addDiscreteSetter(fl, m_info, "Automatic mode", "#HP01LC02RAM_01PPC");
	connect(automaticModeSetter, &DiscreteSignalSetter::valueChanged, this, &TuningMainWindow::automaticModeChanged);

	fl = new QFormLayout;
	vl->addStretch();
	vl->addLayout(fl);

	addDiscreteSetter(fl, m_info, "\"Test mode\" on N-16", "#HP01NR01_01PPC");
	addDiscreteSetter(fl, m_info, "\"Test mode\" on Lin APR", "#HP01LC01_01PPC");

	hl->addWidget(groupBox);

	groupBox = new QGroupBox("Setting coeficients", this);
	groupBox->setStyleSheet("QGroupBox{border:1px solid gray;border-radius:5px;margin-top: 1ex;} QGroupBox::title{subcontrol-origin: margin;subcontrol-position:top center;padding:0 3px;}");
	fl = new QFormLayout;
	fl->setVerticalSpacing(20);
	groupBox->setLayout(fl);
	addAnalogSetter(fl, m_info, "Limit for power error", "#HP01LC01RLR_01PPC", 100);
	addAnalogSetter(fl, m_info, "Scaling coefficient", "#HP01LC02RCC_01PPC", 100);
	addAnalogSetter(fl, m_info, "Driving coefficient", "#HP01LC02RDC_01PPC", 100);
	hl->addWidget(groupBox);

	font = m_automaticPowerRegulatorWidget->font();
	font.setPointSize(font.pointSize() * 1.4);
	setFontRecursive(m_automaticPowerRegulatorWidget, font);
	// ========== Fourth tab ==========
}


void LogWriter::writeFrameToLog(QString caption, FotipFrame& fotipFrame)
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
	writeField(out, "Subsystem key", header.subsystemKey.wordVaue);
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


void LogWriter::onUserRequest(FotipFrame fotipFrame)
{
	writeFrameToLog("User request", fotipFrame);
}


void LogWriter::onReplyWithNoZeroFlags(FotipFrame fotipFrame)
{
	writeFrameToLog("Reply with no zero flags", fotipFrame);
}
