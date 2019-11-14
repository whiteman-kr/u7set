#include "SignalInfoPanel.h"

#include <QApplication>
#include <QMainWindow>
#include <QIcon>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QClipboard>

#include "Options.h"
#include "ObjectProperties.h"
#include "Conversion.h"
#include "CalibratorBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::SignalInfoTable(QObject*)
{
	connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &SignalInfoTable::updateSignalParam, Qt::QueuedConnection);
}

// -------------------------------------------------------------------------------------------------------------------

SignalInfoTable::~SignalInfoTable()
{
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
			result = SignalInfoColumn[section];
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
		return theOptions.signalInfo().font();
	}

	if (role == Qt::TextColorRole)
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

	if (role == Qt::BackgroundColorRole)
	{
		if (column == SIGNAL_INFO_COLUMN_STATE)
		{
			Metrology::SignalState state;

			if (ioParam.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
			{
				state = theSignalBase.signalState(ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT).hash());
			}
			else
			{
				state = theSignalBase.signalState(ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT).hash());
			}

			if (state.flags().valid == false)
			{
				return theOptions.signalInfo().colorFlagValid();
			}

//			if (state.flags().overflow == true)
//			{
//				return theOptions.signalInfo().colorFlagOverflow();
//			}

//			if (state.flags().underflow == true)
//			{
//				return theOptions.signalInfo().colorFlagUnderflow();
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
		Metrology::SignalParam inParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (inParam.isValid() == true)
		{
			Metrology::SignalState inState = theSignalBase.signalState(inParam.hash());
			stateStr = signalStateStr(inParam, inState);
		}

		if (ioParam.signalConnectionType() != SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			Metrology::SignalParam outParam = ioParam.param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
			if (outParam.isValid() == true)
			{
				Metrology::SignalState outState = theSignalBase.signalState(outParam.hash());
				stateStr += MultiTextDivider + signalStateStr(outParam, outState);
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
		case SIGNAL_INFO_COLUMN_EN_RANGE:		result = ioParam.engeneeringRangeStr();	break;
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

	if (state.flags().valid == false)
	{
		return tr("No valid");
	}

	QString stateStr, formatStr;

	formatStr.sprintf(("%%.%df"), param.decimalPlaces());

	stateStr.sprintf(formatStr.toAscii(), state.value());

	if (param.unit().isEmpty() == false)
	{
		stateStr.append(" " + param.unit());
	}

	// append electrical equivalent
	//
	if (theOptions.signalInfo().showElectricState() == true)
	{
		if (param.isInput() == true || param.isOutput() == true)
		{
			double electric = conversion(state.value(), CT_ENGENEER_TO_ELECTRIC, param);
			stateStr.append(" = " + QString::number(electric, 10, param.electricPrecision()));

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

	IoSignalParam ioParam;

	m_signalMutex.lock();

		ioParam = m_ioParamList[index];

	m_signalMutex.unlock();

	return ioParam;
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

void SignalInfoTable::updateSignalParam(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	m_signalMutex.lock();

		int signalCount = m_ioParamList.count();
		for(int c = 0; c < signalCount; c ++)
		{
			for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
			{
				if (m_ioParamList[c].param(type).appSignalID() == appSignalID)
				{
					m_ioParamList[c].setParam(type, theSignalBase.signalParam(appSignalID));
				}
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalInfoPanel::SignalInfoPanel(QWidget* parent) :
	QDockWidget(parent)
{
	setWindowTitle(tr("Panel signal information"));
	setObjectName(windowTitle());

	createInterface();
	createHeaderContexMenu();
	createContextMenu();

	connect(&theSignalBase, &SignalBase::activeSignalChanged, this, &SignalInfoPanel::activeSignalChanged, Qt::QueuedConnection);

	hideColumn(SIGNAL_INFO_COLUMN_CUSTOM_ID, true);
	hideColumn(SIGNAL_INFO_COLUMN_EQUIPMENT_ID, true);
	hideColumn(SIGNAL_INFO_COLUMN_RACK, true);
	hideColumn(SIGNAL_INFO_COLUMN_CHASSIS, true);
	hideColumn(SIGNAL_INFO_COLUMN_MODULE, true);
	hideColumn(SIGNAL_INFO_COLUMN_PLACE, true);
	hideColumn(SIGNAL_INFO_COLUMN_EL_SENSOR, true);

	startSignalStateTimer();
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

	m_pView = new QTableView(m_pSignalInfoWindow);
	m_pView->setModel(&m_signalParamTable);
	QSize cellSize = QFontMetrics(theOptions.signalInfo().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pSignalInfoWindow->setCentralWidget(m_pView);

	for(int column = 0; column < SIGNAL_INFO_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalInfoColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

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
		m_pColumnAction[column] = m_headerContextMenu->addAction(SignalInfoColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalInfoPanel::onColumnAction);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pSignalInfoWindow);

	m_pShowMenu = new QMenu(tr("Show"), m_pSignalInfoWindow);

	m_pShowElectricValueAction = m_pShowMenu->addAction(tr("Electrical state"));
	m_pShowElectricValueAction->setCheckable(true);
	m_pShowElectricValueAction->setChecked(theOptions.signalInfo().showElectricState());

	m_pContextMenu->addMenu(m_pShowMenu);

	m_pContextMenu->addSeparator();

	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pContextMenu->addSeparator();

	m_pSignalPropertyAction = m_pContextMenu->addAction(tr("Properties ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	connect(m_pShowElectricValueAction, &QAction::triggered, this, &SignalInfoPanel::showElectricValue);
	connect(m_pCopyAction, &QAction::triggered, this, &SignalInfoPanel::copy);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &SignalInfoPanel::signalProperty);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &SignalInfoPanel::onContextMenu);
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

void SignalInfoPanel::startSignalStateTimer()
{
	if (m_updateSignalStateTimer == nullptr)
	{
		m_updateSignalStateTimer = new QTimer(this);
		connect(m_updateSignalStateTimer, &QTimer::timeout, this, &SignalInfoPanel::updateSignalState);
	}

	m_updateSignalStateTimer->start(theOptions.signalInfo().timeForUpdate());
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

void SignalInfoPanel::restartSignalStateTimer()
{
	stopSignalStateTimer();
	startSignalStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::onContextMenu(QPoint)
{
	m_pShowElectricValueAction->setChecked(theOptions.signalInfo().showElectricState());

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalInfoPanel::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
		{
			signalProperty();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::activeSignalChanged(const MeasureSignal& activeSignal)
{
	clear();

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

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			Metrology::Signal* pSignal = activeSignal.multiSignal(type).metrologySignal(c);
			if (pSignal == nullptr)
			{
				continue;
			}

			Metrology::SignalParam& param = pSignal->param();
			if (param.isValid() == false)
			{
				continue;
			}

			ioParam.setParam(type, param);
			ioParam.setSignalConnectionType(activeSignal.signalConnectionType());
			ioParam.setCalibratorManager(theCalibratorBase.calibratorForMeasure(c));
		}

		ioParamList.append(ioParam);
	}

	m_signalParamTable.set(ioParamList);

	//
	//
	QSize cellSize = QFontMetrics(theOptions.signalInfo().font()).size(Qt::TextSingleLine,"A");

	if (activeSignal.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
	}
	else
	{
		m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height() * 2);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::updateSignalState()
{
	m_signalParamTable.updateColumn(SIGNAL_INFO_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::showElectricValue()
{
	theOptions.signalInfo().setShowElectricState(m_pShowElectricValueAction->isChecked());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalInfoPanel::copy()
{
	QString textClipboard;

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
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

	if (theOptions.toolBar().signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		param = m_signalParamTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_INPUT);
	}
	else
	{
		param = m_signalParamTable.signalParam(index).param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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
