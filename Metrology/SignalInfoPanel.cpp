#include "SignalInfoPanel.h"

#include <QApplication>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QKeyEvent>

#include "../lib/UnitsConvertor.h"

#include "ProcessData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::SignalInfoTable(QObject*)
{
	connect(&theSignalBase, &SignalBase::signalParamChanged, this, &SignalInfoTable::signalParamChanged, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::~SignalInfoTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::setSignalInfo(const SignalInfoOption& signalInfo)
{
	m_signalInfo = signalInfo;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::columnCount(const QModelIndex&) const
{
	return SIGNAL_INFO_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalInfoTable::rowCount(const QModelIndex&) const
{
	return m_signalCount;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalInfoTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SIGNAL_INFO_COLUMN_COUNT)
		{
			result = qApp->translate("SignalInfoMeasure.h", SignalInfoColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalInfoTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= m_signalCount)
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
	{
		return QVariant();
	}

	IoSignalParam ioParam = signalParam(row);
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

			if (ioParam.connectionType() == Metrology::ConnectionType::Unsed)
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
	if (column < 0 || column > SIGNAL_INFO_COLUMN_COUNT)
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

		if (ioParam.connectionType() != Metrology::ConnectionType::Unsed)
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
			return qApp->translate("MeasureSignal.h", Metrology::SignalNoValid);
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

void SignalInfoTable::updateColumn(int column)
{
	if (column < 0 || column >= SIGNAL_INFO_COLUMN_COUNT)
	{
		return;
	}

	for (int row = 0; row < m_signalCount; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam SignalInfoTable::signalParam(int index) const
{
	if (index < 0 || index >= m_signalCount)
	{
		return IoSignalParam();
	}

	QMutexLocker l(&m_signalMutex);

	return m_ioParamList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::set(const QVector<IoSignalParam>& ioParamList)
{
	int signalCount = ioParamList.count();
	if (signalCount == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, signalCount - 1);

		m_signalMutex.lock();

			m_ioParamList = ioParamList;
			m_signalCount = signalCount;

		m_signalMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::clear()
{
	if (m_signalCount == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, m_signalCount - 1);

		m_signalMutex.lock();

			m_signalCount = 0;
			m_ioParamList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoTable::signalParamChanged(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QMutexLocker l(&m_signalMutex);

	int signalCount = m_ioParamList.count();
	for(int c = 0; c < signalCount; c ++)
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			if (m_ioParamList[c].param(ioType).appSignalID() == appSignalID)
			{
				m_ioParamList[c].setParam(ioType, theSignalBase.signalParam(appSignalID));
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoPanel::SignalInfoPanel(const SignalInfoOption& signalInfo, QWidget* parent) :
	QDockWidget(parent),
	m_signalInfo(signalInfo)
{
	setWindowTitle(tr("Panel signal information"));
	setObjectName(windowTitle());

	createInterface();
	createHeaderContexMenu();
	initContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged,
			this, &SignalInfoPanel::activeSignalChanged, Qt::QueuedConnection);

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

SignalInfoPanel::~SignalInfoPanel()
{
	stopSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createInterface()
{
	m_pSignalInfoWindow = new QMainWindow;

	m_pSignalInfoWindow->installEventFilter(this);

	m_signalParamTable.setSignalInfo(m_signalInfo);

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

	connect(m_pView, &QTableView::doubleClicked , this, &SignalInfoPanel::onListDoubleClicked);

	setWidget(m_pSignalInfoWindow);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &SignalInfoPanel::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("SignalInfoMeasure.h", SignalInfoColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalInfoPanel::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::initContextMenu()
{
	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalInfoPanel::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createContextMenu()
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
	if (m_measureKind != MEASURE_KIND_ONE_RACK)
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

	if (m_measureKind == MEASURE_KIND_ONE_RACK)
	{
		if (m_signalParamTable.signalCount() > 1)
		{
			m_pShowMenu->addSeparator();

			if ( m_pView->currentIndex().row() > 0)
			{
				m_pShowSignalMoveUpAction = m_pShowMenu->addAction(tr("Move Up"));
				connect(m_pShowSignalMoveUpAction, &QAction::triggered, this, &SignalInfoPanel::showSignalMoveUp);
			}

			if ( m_pView->currentIndex().row() < m_signalParamTable.signalCount() - 1)
			{
				m_pShowSignalMoveDownAction = m_pShowMenu->addAction(tr("Move Down"));
				connect(m_pShowSignalMoveDownAction, &QAction::triggered, this, &SignalInfoPanel::showSignalMoveDown);
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

	connect(m_pShowNoValidAction, &QAction::triggered, this, &SignalInfoPanel::showNoValid);
	connect(m_pShowElectricValueAction, &QAction::triggered, this, &SignalInfoPanel::showElectricValue);
	connect(m_pCopyAction, &QAction::triggered, this, &SignalInfoPanel::copy);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalInfoPanel::signalProperty);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::appendMetrologyConnetionMenu()
{
	if (m_connectionType == Metrology::ConnectionType::Unsed)
	{
		return;
	}

	if (m_pContextMenu == nullptr)
	{
		return;
	}

	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.signalCount())
	{
		return;
	}

	m_pConnectionActionList.clear();
	m_destSignals.clear();

	const IoSignalParam& ioParam = m_signalParamTable.signalParam(index);

	const Metrology::SignalParam& sourParam = ioParam.param(Metrology::ConnectionIoType::Source);
	const Metrology::SignalParam& destParam = ioParam.param(Metrology::ConnectionIoType::Destination);

	if (sourParam.isValid() == false || destParam.isValid() == false)
	{
		return;
	}

	m_destSignals = theSignalBase.connections().destinationSignals(m_connectionType, sourParam.appSignalID());

	int destSignalCount = m_destSignals.count();
	for (int s = 0; s < destSignalCount; s++)
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

			m_pConnectionActionList.append(pConnctionAction);
		}
	}

	if (m_pConnectionActionList.isEmpty() == true)
	{
		return;
	}

	connect(m_pContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),
			this, &SignalInfoPanel::onConnectionAction);

	m_pContextMenu->addSeparator();
}


// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::hideColumn(int column, bool hide)
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

void SignalInfoPanel::startSignalStateTimer(int timeout)
{
	if (m_updateSignalStateTimer == nullptr)
	{
		m_updateSignalStateTimer = new QTimer(this);
		connect(m_updateSignalStateTimer, &QTimer::timeout, this, &SignalInfoPanel::updateSignalState);
	}

	m_updateSignalStateTimer->start(timeout);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::stopSignalStateTimer()
{
	if (m_updateSignalStateTimer != nullptr)
	{
		m_updateSignalStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::restartSignalStateTimer(int timeout)
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

void SignalInfoPanel::setSignalInfo(const SignalInfoOption& signalInfo)
{
	m_signalInfo = signalInfo;
	m_signalParamTable.setSignalInfo(m_signalInfo);
	restartSignalStateTimer(m_signalInfo.timeForUpdate());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onContextMenu(QPoint)
{
	createContextMenu();

	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalInfoPanel::eventFilter(QObject* object, QEvent* event)
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

void SignalInfoPanel::measureKindChanged(int kind)
{
	if (kind < 0 || kind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	m_measureKind = kind;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::connectionTypeChanged(Metrology::ConnectionType type)
{
	if (static_cast<int>(type) < 0 || static_cast<int>(type) >= Metrology::ConnectionTypeCount)
	{
		return;
	}

	m_connectionType = type;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::activeSignalChanged(const MeasureSignal& activeSignal)
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

	QVector<IoSignalParam> ioParamList;

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

		ioParamList.append(ioParam);
	}

	m_signalParamTable.set(ioParamList);

	//
	//
	QSize cellSize = QFontMetrics(m_signalInfo.font()).size(Qt::TextSingleLine,"A");

	if (activeSignal.connectionType() == Metrology::ConnectionType::Unsed)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(static_cast<int>(cellSize.height() * 2.3));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateSignalState()
{
	m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onConnectionAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	int destSignalIndex = -1;

	int connectionActionCount = m_pConnectionActionList.count();
	for(int i = 0; i < connectionActionCount; i++)
	{
		if (m_pConnectionActionList[i] == action)
		{
			action->setChecked(true);

			destSignalIndex = i;

			break;
		}
	}

	if (destSignalIndex < 0 || destSignalIndex >= m_destSignals.count())
	{
		return;
	}

	Metrology::Signal* pDestSignal = m_destSignals[destSignalIndex];
	if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
	{
		return;
	}

	int channel = m_pView->currentIndex().row();
	if (channel < 0 || channel >= m_signalParamTable.signalCount())
	{
		return;
	}

	emit changeActiveDestSignal(channel, pDestSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showNoValid()
{
	m_signalInfo.setShowNoValid(m_pShowNoValidAction->isChecked());
	m_signalParamTable.setSignalInfo(m_signalInfo);
	m_signalInfo.save();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showElectricValue()
{
	m_signalInfo.setShowElectricState(m_pShowElectricValueAction->isChecked());
	m_signalParamTable.setSignalInfo(m_signalInfo);
	m_signalInfo.save();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showSignalMoveUp()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.signalCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select signal for move!"));
		return;
	}

	IoSignalParam ioParam = m_signalParamTable.signalParam(index);
	if (ioParam.isValid() == false)
	{
		return;
	}

	int indexPrev = index - 1;
	if (indexPrev < 0 || indexPrev >= m_signalParamTable.signalCount())
	{
		return;
	}

	IoSignalParam ioParamPrev = m_signalParamTable.signalParam(indexPrev);
	if (ioParamPrev.isValid() == false)
	{
		return;
	}

	emit changeActiveDestSignals(index, indexPrev);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showSignalMoveDown()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.signalCount())
	{
		QMessageBox::information(this, windowTitle(), tr("Please, select signal for move!"));
		return;
	}

	IoSignalParam ioParam = m_signalParamTable.signalParam(index);
	if (ioParam.isValid() == false)
	{
		return;
	}

	int indexNext = index + 1;
	if (indexNext < 0 || indexNext >= m_signalParamTable.signalCount())
	{
		return;
	}

	IoSignalParam ioParamNext = m_signalParamTable.signalParam(indexNext);
	if (ioParamNext.isValid() == false)
	{
		return;
	}

	emit changeActiveDestSignals(index, indexNext);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::signalProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_signalParamTable.signalCount())
	{
		return;
	}

	Metrology::SignalParam param;

	if (m_connectionType == Metrology::ConnectionType::Unsed)
	{
		param = m_signalParamTable.signalParam(index).param(Metrology::ConnectionIoType::Source);
	}
	else
	{
		param = m_signalParamTable.signalParam(index).param(Metrology::ConnectionIoType::Destination);
	}

	if (param.isValid() == false)
	{
		return;
	}

	SignalPropertyDialog dialog(param);
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onColumnAction(QAction* action)
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
