#include "PanelSignalInfo.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "UnitsConvertor.h"
#include "ProcessData.h"
#include "DialogObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant SignalInfoTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
	{
		return QVariant();
	}

	IoSignalParam ioParam = at(row);
	if (ioParam.isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case SIGNAL_INFO_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case SIGNAL_INFO_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case SIGNAL_INFO_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case SIGNAL_INFO_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case SIGNAL_INFO_COLUMN_STATE:			result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_RACK:			result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_CHASSIS:		result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_MODULE:			result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_PLACE:			result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_EL_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_EL_SENSOR:		result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_EN_RANGE:		result = Qt::AlignCenter;	break;
			case SIGNAL_INFO_COLUMN_CALIBRATOR:		result = Qt::AlignCenter;	break;
			default:								assert(0);
		}

		return result;
	}

	if (role == Qt::FontRole)
	{
		return m_signalInfo.font();
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == SIGNAL_INFO_COLUMN_CALIBRATOR)
		{
			if (ioParam.calibratorManager() == nullptr || ioParam.calibratorManager()->calibratorIsConnected() == false)
			{
				return QColor(Qt::lightGray);
			}
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == SIGNAL_INFO_COLUMN_STATE)
		{
			Metrology::SignalState state;

			if (ioParam.connectionType() == Metrology::ConnectionType::Unused)
			{
				state = theSignalBase.signalState(ioParam.param(Metrology::ConnectionIoType::Source).hash());
			}
			else
			{
				state = theSignalBase.signalState(ioParam.param(Metrology::ConnectionIoType::Destination).hash());
			}

			if (state.flags().valid == false)
			{
				return m_signalInfo.colorFlagValid();
			}

//			if (state.flags().overflow == true)
//			{
//				return m_signalInfo.colorFlagOverflow();
//			}

//			if (state.flags().underflow == true)
//			{
//				return m_signalInfo.colorFlagUnderflow();
//			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(column, ioParam);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::text(int column, const IoSignalParam& ioParam) const
{
	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	if (ioParam.isValid() == false)
	{
		return QString();
	}

	QString stateStr;

	if (column == SIGNAL_INFO_COLUMN_STATE)
	{
		const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
		if (inParam.isValid() == true)
		{
			const Metrology::SignalState& inState = theSignalBase.signalState(inParam.hash());
			stateStr = signalStateStr(inParam, inState);
		}

		if (ioParam.connectionType() != Metrology::ConnectionType::Unused)
		{
			const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
			if (outParam.isValid() == true)
			{
				const Metrology::SignalState& outState = theSignalBase.signalState(outParam.hash());
				stateStr += MULTI_TEXT_DEVIDER + signalStateStr(outParam, outState);
			}
		}
	}

	QString result;

	switch (column)
	{
		case SIGNAL_INFO_COLUMN_APP_ID:			result = ioParam.appSignalID();			break;
		case SIGNAL_INFO_COLUMN_CUSTOM_ID:		result = ioParam.customSignalID();		break;
		case SIGNAL_INFO_COLUMN_EQUIPMENT_ID:	result = ioParam.equipmentID();			break;
		case SIGNAL_INFO_COLUMN_CAPTION:		result = ioParam.caption();				break;
		case SIGNAL_INFO_COLUMN_STATE:			result = stateStr;						break;
		case SIGNAL_INFO_COLUMN_RACK:			result = ioParam.rackCaption();			break;
		case SIGNAL_INFO_COLUMN_CHASSIS:		result = ioParam.chassisStr();			break;
		case SIGNAL_INFO_COLUMN_MODULE:			result = ioParam.moduleStr();			break;
		case SIGNAL_INFO_COLUMN_PLACE:			result = ioParam.placeStr();			break;
		case SIGNAL_INFO_COLUMN_EL_RANGE:		result = ioParam.electricRangeStr();	break;
		case SIGNAL_INFO_COLUMN_EL_SENSOR:		result = ioParam.electricSensorStr();	break;
		case SIGNAL_INFO_COLUMN_EN_RANGE:		result = ioParam.engineeringRangeStr();	break;
		case SIGNAL_INFO_COLUMN_CALIBRATOR:		result = ioParam.calibratorStr();		break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalInfoTable::signalStateStr(const Metrology::SignalParam& param, const Metrology::SignalState& state) const
{
	if (param.isValid() == false)
	{
		return QString();
	}

	if (m_signalInfo.showNoValid() == false)
	{
		if (state.flags().valid == false)
		{
			return qApp->translate("MetrologySignal", Metrology::SignalNoValid);
		}
	}

	QString stateStr = QString::number(state.value(), 'f', param.decimalPlaces());

	if (param.unit().isEmpty() == false)
	{
		stateStr.append(" " + param.unit());
	}

	// append electrical equivalent
	//
	if (m_signalInfo.showElectricState() == true)
	{
		if (param.isInput() == true || param.isOutput() == true)
		{
			UnitsConvertor uc;
			double electric = uc.conversion(state.value(), UnitsConvertType::PhysicalToElectric, param);

			stateStr.append(" = " + QString::number(electric, 'f', param.electricPrecision()));

			if (param.electricUnitStr().isEmpty() == false)
			{
				stateStr.append(" " + param.electricUnitStr());
			}
		}
	}

	// check flags
	//
//	if (state.flags().underflow == true)
//	{
//		stateStr.append(" - Underflow");
//	}

//	if (state.flags().overflow == true)
//	{
//		stateStr.append(" - Overflow");
//	}

	return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::signalParamChanged(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QMutexLocker l(&m_mutex);

	quint64 signalCount = m_list.size();
	for(quint64 c = 0; c < signalCount; c ++)
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			if (m_list[c].param(ioType).appSignalID() == appSignalID)
			{
				m_list[c].setParam(ioType, theSignalBase.signalParam(appSignalID));
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PanelSignalInfo::PanelSignalInfo(const SignalInfoOption& signalInfo, QWidget* parent) :
	QDockWidget(parent),
	m_signalInfo(signalInfo)
{
	setWindowTitle(tr("Panel signal information"));
	setObjectName(windowTitle());

	createInterface();
	createHeaderContexMenu();
	initContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged,
			this, &PanelSignalInfo::activeSignalChanged, Qt::QueuedConnection);

	hideColumn(SIGNAL_INFO_COLUMN_CUSTOM_ID, true);
	hideColumn(SIGNAL_INFO_COLUMN_EQUIPMENT_ID, true);
	hideColumn(SIGNAL_INFO_COLUMN_RACK, true);
	hideColumn(SIGNAL_INFO_COLUMN_CHASSIS, true);
	hideColumn(SIGNAL_INFO_COLUMN_MODULE, true);
	hideColumn(SIGNAL_INFO_COLUMN_PLACE, true);
	hideColumn(SIGNAL_INFO_COLUMN_EL_SENSOR, true);

	startSignalStateTimer(m_signalInfo.timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

PanelSignalInfo::~PanelSignalInfo()
{
	stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::createInterface()
{
	m_pSignalInfoWindow = new QMainWindow;

	m_pSignalInfoWindow->installEventFilter(this);

	m_signalParamTable.setColumnCaption(PanelSignalInfo::metaObject()->className(), SIGNAL_INFO_COLUMN_COUNT, SignalInfoColumn);
	m_signalParamTable.setSignalInfo(m_signalInfo);
	connect(&theSignalBase, &SignalBase::signalParamChanged, &m_signalParamTable, &SignalInfoTable::signalParamChanged, Qt::QueuedConnection);

	m_pView = new QTableView(m_pSignalInfoWindow);
	m_pView->setModel(&m_signalParamTable);
	QSize cellSize = QFontMetrics(m_signalInfo.font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pSignalInfoWindow->setCentralWidget(m_pView);

	for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalInfoColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &PanelSignalInfo::onListDoubleClicked);

	setWidget(m_pSignalInfoWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &PanelSignalInfo::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("PanelSignalInfo", SignalInfoColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &PanelSignalInfo::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::initContextMenu()
{
	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &PanelSignalInfo::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::createContextMenu()
{
	if (m_pContextMenu != nullptr)
	{
		delete m_pContextMenu;
		m_pContextMenu = nullptr;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pSignalInfoWindow);

	// append metrology connection items
	//
	if (m_measureKind != Measure::Kind::OneRack)
	{
		appendMetrologyConnetionMenu();
	}

	//
	//
	m_pShowMenu = new QMenu(tr("Show"), m_pSignalInfoWindow);

	m_pShowNoValidAction = m_pShowMenu->addAction(tr("State if signal is no valid"));
	m_pShowNoValidAction->setCheckable(true);
	m_pShowNoValidAction->setChecked(m_signalInfo.showNoValid());

	m_pShowElectricValueAction = m_pShowMenu->addAction(tr("Electrical state"));
	m_pShowElectricValueAction->setCheckable(true);
	m_pShowElectricValueAction->setChecked(m_signalInfo.showElectricState());

	if (m_measureKind == Measure::Kind::OneRack)
	{
		if (m_signalParamTable.count() > 1)
		{
			m_pShowMenu->addSeparator();

			if ( m_pView->currentIndex().row() > 0)
			{
				m_pShowSignalMoveUpAction = m_pShowMenu->addAction(tr("Move Up"));
				connect(m_pShowSignalMoveUpAction, &QAction::triggered, this, &PanelSignalInfo::showSignalMoveUp);
			}

			if ( m_pView->currentIndex().row() < m_signalParamTable.count() - 1)
			{
				m_pShowSignalMoveDownAction = m_pShowMenu->addAction(tr("Move Down"));
				connect(m_pShowSignalMoveDownAction, &QAction::triggered, this, &PanelSignalInfo::showSignalMoveDown);
			}
		}
	}

	m_pContextMenu->addMenu(m_pShowMenu);

	m_pContextMenu->addSeparator();

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pContextMenu->addSeparator();

	m_pSignalPropertyAction = m_pContextMenu->addAction(tr("Propertу ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	connect(m_pShowNoValidAction, &QAction::triggered, this, &PanelSignalInfo::showNoValid);
	connect(m_pShowElectricValueAction, &QAction::triggered, this, &PanelSignalInfo::showElectricValue);
	connect(m_pCopyAction, &QAction::triggered, this, &PanelSignalInfo::copy);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &PanelSignalInfo::signalProperty);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::appendMetrologyConnetionMenu()
{
	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		return;
	}

	if (m_pContextMenu == nullptr)
	{
		return;
	}

	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.count())
	{
		return;
	}

	m_pConnectionActionList.clear();
	m_destSignals.clear();

	const IoSignalParam& ioParam = m_signalParamTable.at(index);

	const Metrology::SignalParam& sourParam = ioParam.param(Metrology::ConnectionIoType::Source);
	const Metrology::SignalParam& destParam = ioParam.param(Metrology::ConnectionIoType::Destination);

	if (sourParam.isValid() == false || destParam.isValid() == false)
	{
		return;
	}

	m_destSignals = theSignalBase.connections().destinationSignals(sourParam.appSignalID(), m_connectionType);

	quint64 destSignalCount = m_destSignals.size();
	for (quint64 s = 0; s < destSignalCount; s++)
	{
		Metrology::Signal* pDestinationSignal = m_destSignals[s];
		if (pDestinationSignal == nullptr || pDestinationSignal->param().isValid() == false)
		{
			continue;
		}

		QString strConnection = sourParam.appSignalID() + " -> " + pDestinationSignal->param().appSignalID();

		QAction* pConnctionAction = m_pContextMenu->addAction(strConnection);
		if (pConnctionAction != nullptr)
		{
			pConnctionAction->setCheckable(true);
			pConnctionAction->setChecked(false);

			if (destParam.appSignalID() == pDestinationSignal->param().appSignalID())
			{
				pConnctionAction->setChecked(true);
			}

			m_pConnectionActionList.push_back(pConnctionAction);
		}
	}

	if (m_pConnectionActionList.size() == 0)
	{
		return;
	}

	connect(m_pContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),
			this, &PanelSignalInfo::onConnectionAction);

	m_pContextMenu->addSeparator();
}


// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= SIGNAL_INFO_COLUMN_COUNT)
	{
		return;
	}

	if (hide == true)
	{
		m_pView->hideColumn(column);
		m_pColumnAction[column]->setChecked(false);
	}
	else
	{
		m_pView->showColumn(column);
		m_pColumnAction[column]->setChecked(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::startSignalStateTimer(int timeout)
{
	if (m_updateSignalStateTimer == nullptr)
	{
		m_updateSignalStateTimer = new QTimer(this);
		connect(m_updateSignalStateTimer, &QTimer::timeout, this, &PanelSignalInfo::updateSignalState);
	}

	m_updateSignalStateTimer->start(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::stopSignalStateTimer()
{
	if (m_updateSignalStateTimer != nullptr)
	{
		m_updateSignalStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::restartSignalStateTimer(int timeout)
{
	if (m_updateSignalStateTimer != nullptr)
	{
		if(m_updateSignalStateTimer->interval() == timeout)
		{
			return;
		}
	}

	stopSignalStateTimer();
	startSignalStateTimer(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::setSignalInfo(const SignalInfoOption& signalInfo)
{
	m_signalInfo = signalInfo;
	m_signalParamTable.setSignalInfo(m_signalInfo);
	restartSignalStateTimer(m_signalInfo.timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::onContextMenu(QPoint)
{
	createContextMenu();

	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool PanelSignalInfo::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent* >(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			signalProperty();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::measureKindChanged(Measure::Kind measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = measureKind;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::connectionTypeChanged(Metrology::ConnectionType connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	m_connectionType = connectionType;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::activeSignalChanged(const MeasureSignal& activeSignal)
{
	clear();

	if (m_pCalibratorBase == nullptr)
	{
		return;
	}

	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	int signalCount = activeSignal.channelCount();
	if (signalCount == 0)
	{
		return;
	}

	std::vector<IoSignalParam> ioParamList;

	for(int c = 0; c < signalCount; c ++)
	{
		IoSignalParam ioParam;

		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			Metrology::Signal* pSignal = activeSignal.multiChannelSignal(ioType).metrologySignal(c);
			if (pSignal == nullptr)
			{
				continue;
			}

			Metrology::SignalParam& param = pSignal->param();
			if (param.isValid() == false)
			{
				continue;
			}

			ioParam.setParam(ioType, param);
			ioParam.setConnectionType(activeSignal.connectionType());
			ioParam.setCalibratorManager(m_pCalibratorBase->calibratorForMeasure(c));
		}

		ioParamList.push_back(ioParam);
	}

	m_signalParamTable.set(ioParamList);

	//
	//
	QSize cellSize = QFontMetrics(m_signalInfo.font()).size(Qt::TextSingleLine,"A");

	if (activeSignal.connectionType() == Metrology::ConnectionType::Unused)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(static_cast<int>(cellSize.height() * 2.3));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::updateSignalState()
{
	m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::onConnectionAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	quint64 destSignalIndex = ULLONG_MAX;

	quint64 connectionActionCount = m_pConnectionActionList.size();
	for(quint64 i = 0; i < connectionActionCount; i++)
	{
		if (m_pConnectionActionList[i] == action)
		{
			action->setChecked(true);

			destSignalIndex = i;

			break;
		}
	}

	if (destSignalIndex >= m_destSignals.size())
	{
		return;
	}

	Metrology::Signal* pDestSignal = m_destSignals[destSignalIndex];
	if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
	{
		return;
	}

	int channel = m_pView->currentIndex().row();
	if (channel < 0 || channel >= m_signalParamTable.count())
	{
		return;
	}

	emit changeActiveDestSignal(channel, pDestSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::showNoValid()
{
	m_signalInfo.setShowNoValid(m_pShowNoValidAction->isChecked());
	m_signalParamTable.setSignalInfo(m_signalInfo);
	m_signalInfo.save();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::showElectricValue()
{
	m_signalInfo.setShowElectricState(m_pShowElectricValueAction->isChecked());
	m_signalParamTable.setSignalInfo(m_signalInfo);
	m_signalInfo.save();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::showSignalMoveUp()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.count())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select signal for move!"));
		return;
	}

	IoSignalParam ioParam = m_signalParamTable.at(index);
	if (ioParam.isValid() == false)
	{
		return;
	}

	int indexPrev = index - 1;
	if (indexPrev < 0 || indexPrev >= m_signalParamTable.count())
	{
		return;
	}

	IoSignalParam ioParamPrev = m_signalParamTable.at(indexPrev);
	if (ioParamPrev.isValid() == false)
	{
		return;
	}

	emit changeActiveDestSignals(index, indexPrev);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::showSignalMoveDown()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.count())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select signal for move!"));
		return;
	}

	IoSignalParam ioParam = m_signalParamTable.at(index);
	if (ioParam.isValid() == false)
	{
		return;
	}

	int indexNext = index + 1;
	if (indexNext < 0 || indexNext >= m_signalParamTable.count())
	{
		return;
	}

	IoSignalParam ioParamNext = m_signalParamTable.at(indexNext);
	if (ioParamNext.isValid() == false)
	{
		return;
	}

	emit changeActiveDestSignals(index, indexNext);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::signalProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.count())
	{
		return;
	}

	Metrology::SignalParam param;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		param = m_signalParamTable.at(index).param(Metrology::ConnectionIoType::Source);
	}
	else
	{
		param = m_signalParamTable.at(index).param(Metrology::ConnectionIoType::Destination);
	}

	if (param.isValid() == false)
	{
		return;
	}

	DialogSignalProperty dialog(param, this);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelSignalInfo::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
	{
		if (m_pColumnAction[column] == action)
		{
			hideColumn(column, !action->isChecked());

			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
