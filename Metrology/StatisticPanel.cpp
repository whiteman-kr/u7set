#include "StatisticPanel.h"

#include <QClipboard>
#include <QHeaderView>

#include "MainWindow.h"
#include "Delegate.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticTable::StatisticTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::~StatisticTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::columnCount(const QModelIndex&) const
{
	return STATISTIC_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::rowCount(const QModelIndex&) const
{
	return m_statisticItemCount;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < STATISTIC_COLUMN_COUNT)
		{
			result = qApp->translate("StatisticDialog.h", StatisticColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= m_statisticItemCount)
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > STATISTIC_COLUMN_COUNT)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case STATISTIC_COLUMN_APP_ID:				result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CUSTOM_ID:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_EQUIPMENT_ID:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CAPTION:				result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CMP_VALUE:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CMP_NO:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_CMP_OUT_ID:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_RACK:					result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_CHASSIS:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MODULE:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_PLACE:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_EL_RANGE:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_EL_SENSOR:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_EN_RANGE:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_SIGNAL_TYPE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_SIGNAL_CONNECTION:	result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MEASURE_COUNT:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_STATE:				result = Qt::AlignCenter;	break;

			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	const StatisticItem& si = theSignalBase.statistic().item(row);

	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::ForegroundRole)
	{
		if (si.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNDEFINED)
		{
			if (column == STATISTIC_COLUMN_SIGNAL_TYPE || column == STATISTIC_COLUMN_SIGNAL_CONNECTION)
			{
				if (si.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNDEFINED)
				{
					return QColor(Qt::red);
				}
			}
			else
			{
				return QColor(Qt::lightGray);
			}
		}

//		if (pSignal->param().isInput() || pSignal->param().isOutput() == true)
//		{
//			if (pSignal->param().electricRangeIsValid() == false)
//			{
//				return QColor(Qt::lightGray);
//			}
//		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (si.isMeasured() == true)
		{
			switch (si.state())
			{
				case StatisticItem::State::Failed:	return theOptions.measureView().colorErrorLimit();
				case StatisticItem::State::Success:	return theOptions.measureView().colorNotError();
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, si);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::set()
{
	m_statisticItemCount = theSignalBase.statistic().count();
	if (m_statisticItemCount == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, m_statisticItemCount - 1);
	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::clear()
{
	if (m_statisticItemCount == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, m_statisticItemCount - 1);
	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticTable::text(int row, int column, const StatisticItem& si) const
{
	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr)
	{
		return QString();
	}

	const Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	QString comparatorValue;
	QString comparatorNo;
	QString comparatorOutputID;

	if (theSignalBase.statistic().measureType() == MEASURE_TYPE_COMPARATOR)
	{
		std::shared_ptr<Metrology::ComparatorEx> comparator = si.comparator();
		if (comparator != nullptr)
		{
			comparatorValue = comparator->compareDefaultValueStr();
			comparatorNo = QString::number(comparator->index() + 1);
			comparatorOutputID = comparator->output().appSignalID();
		}
	}

	bool visible = true;

	if (row > 0 && theSignalBase.statistic().item(row - 1).signal() == pSignal)
	{
		visible = false;
	}

	QString result;

	switch (column)
	{
		case STATISTIC_COLUMN_APP_ID:				result = visible ? param.appSignalID() : QString();										break;
		case STATISTIC_COLUMN_CUSTOM_ID:			result = visible ? param.customAppSignalID() : QString();								break;
		case STATISTIC_COLUMN_EQUIPMENT_ID:			result = visible ? param.equipmentID() : QString();										break;
		case STATISTIC_COLUMN_CAPTION:				result = visible ? param.caption() : QString();											break;
		case STATISTIC_COLUMN_CMP_VALUE:			result = comparatorValue;																break;
		case STATISTIC_COLUMN_CMP_NO:				result = comparatorNo;																	break;
		case STATISTIC_COLUMN_CMP_OUT_ID:			result = comparatorOutputID;															break;
		case STATISTIC_COLUMN_RACK:					result = visible ? param.location().rack().caption() : QString();						break;
		case STATISTIC_COLUMN_CHASSIS:				result = visible ? param.location().chassisStr() : QString();							break;
		case STATISTIC_COLUMN_MODULE:				result = visible ? param.location().moduleStr() : QString();							break;
		case STATISTIC_COLUMN_PLACE:				result = visible ? param.location().placeStr() : QString();								break;
		case STATISTIC_COLUMN_EL_RANGE:				result = param.electricRangeStr();														break;
		case STATISTIC_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();													break;
		case STATISTIC_COLUMN_EN_RANGE:				result = param.engineeringRangeStr();													break;
		case STATISTIC_COLUMN_SIGNAL_TYPE:			result = param.signalTypeStr();															break;
		case STATISTIC_COLUMN_SIGNAL_CONNECTION:	result = qApp->translate("StatisticBase.cpp", si.signalConnectionTypeStr().toUtf8());	break;
		case STATISTIC_COLUMN_MEASURE_COUNT:		result = si.measureCountStr();															break;
		case STATISTIC_COLUMN_STATE:				result = qApp->translate("StatisticBase.cpp", si.stateStr().toUtf8());					break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::updateSignal(Hash signalHash)
{
	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	QVector<int> rowList;

	int count = theSignalBase.statistic().count();
	for(int i = 0; i < count; i++)
	{
		const StatisticItem& si = theSignalBase.statistic().item(i);

		Metrology::Signal* pSignal = si.signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		if (signalHash == pSignal->param().hash())
		{
			rowList.append(i);
		}
	}

	int rowCount = rowList.count();
	for(int i = 0; i < rowCount; i++)
	{
		int row1 = rowList[i];

		for (int column = 0; column < STATISTIC_COLUMN_COUNT; column ++)
		{
			QModelIndex cellIndex = index(row1, column);

			emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int StatisticPanel::m_measureType = MEASURE_TYPE_LINEARITY;

// -------------------------------------------------------------------------------------------------------------------

StatisticPanel::StatisticPanel(QWidget* parent) :
	QDockWidget(parent)
{
	m_pMainWindow = dynamic_cast<QMainWindow*> (parent);
	if (m_pMainWindow == nullptr)
	{
		return;
	}

	setWindowTitle(tr("Panel statistics (Checklist)"));
	setObjectName(windowTitle());

	createInterface();
	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

StatisticPanel::~StatisticPanel()
{
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::createInterface()
{
	m_pStatisticWindow = new QMainWindow;
	if (m_pStatisticWindow == nullptr)
	{
		return;
	}

	m_pStatisticWindow->installEventFilter(this);

	//
	//
	m_pMenuBar = new QMenuBar(m_pStatisticWindow);
	m_pSignalMenu = new QMenu(tr("&Results"), m_pStatisticWindow);
	m_pEditMenu = new QMenu(tr("&Edit"), m_pStatisticWindow);
	m_pViewMenu = new QMenu(tr("&View"), m_pStatisticWindow);

	m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));

	m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

	m_pEditMenu->addSeparator();

	m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Propertу ..."));
	m_pSignalPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pEditMenu->addSeparator();

	m_pViewGotoMenu = new QMenu(tr("Go to next"), m_pStatisticWindow);
	m_pGotoNextNotMeasuredAction = m_pViewGotoMenu->addAction(tr("Not measured"));
	m_pGotoNextInvalidAction = m_pViewGotoMenu->addAction(tr("Invalid"));

	m_pViewMenu->addMenu(m_pViewGotoMenu);
	m_pViewMenu->addSeparator();

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pExportAction, &QAction::triggered, this, &StatisticPanel::exportSignal);

	connect(m_pFindAction, &QAction::triggered, this, &StatisticPanel::find);
	connect(m_pCopyAction, &QAction::triggered, this, &StatisticPanel::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &StatisticPanel::selectAll);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &StatisticPanel::onProperty);

	connect(m_pGotoNextNotMeasuredAction, &QAction::triggered, this, &StatisticPanel::gotoNextNotMeasured);
	connect(m_pGotoNextInvalidAction, &QAction::triggered, this, &StatisticPanel::gotoNextInvalid);

	m_pStatisticWindow->setMenuBar(m_pMenuBar);

	//
	//
	m_pView = new QTableView(m_pStatisticWindow);
	m_pView->setModel(&m_signalTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, StatisticColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &StatisticPanel::onListDoubleClicked);

	m_pStatisticWindow->setCentralWidget(m_pView);

	setWidget(m_pStatisticWindow);

	createStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::createHeaderContexMenu()
{
	if (m_pView == nullptr)
	{
		return;
	}

	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &StatisticPanel::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("StatisticDialog.h", StatisticColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &StatisticPanel::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::createContextMenu()
{
	if (m_pStatisticWindow == nullptr)
	{
		return;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pStatisticWindow);

	m_pSelectSignalForMeasure = m_pContextMenu->addAction(tr("&Select signal for measuring"));
	m_pFindSignalInMeasureList = m_pContextMenu->addAction(tr("&Find signal in the measure list"));
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSignalPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &StatisticPanel::onContextMenu);

	connect(m_pSelectSignalForMeasure, &QAction::triggered, this, &StatisticPanel::selectSignalForMeasure);
	connect(m_pFindSignalInMeasureList, &QAction::triggered, this, &StatisticPanel::findSignalInMeasureList);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::createStatusBar()
{
	if (m_pStatisticWindow == nullptr)
	{
		return;
	}

	m_pStatusBar = m_pStatisticWindow->statusBar();

	m_statusEmpty = new QLabel(m_pStatusBar);
	m_statusMeasureInavlid = new QLabel(m_pStatusBar);
	m_statusMeasured = new QLabel(m_pStatusBar);

	m_pStatusBar->addWidget(m_statusMeasured);
	m_pStatusBar->addWidget(m_statusMeasureInavlid);
	m_pStatusBar->addWidget(m_statusEmpty);

	m_statusMeasureInavlid->setFixedWidth(100);
	m_statusMeasured->setFixedWidth(150);

	m_pStatusBar->setLayoutDirection(Qt::RightToLeft);
	m_pStatusBar->setSizeGripEnabled(false);
}

// -------------------------------------------------------------------------------------------------------------------

bool StatisticPanel::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			selectSignalForMeasure();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::changedMeasureType(int type)
{
	if (type < 0 || type >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	m_measureType = type;

	theSignalBase.statistic().setMeasureType(m_measureType);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::changedSignalConnectionType(int type)
{
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::activeSignalChanged(const MeasureSignal& activeSignal)
{
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	int signalCount = activeSignal.channelCount();
	if (signalCount == 0)
	{
		return;
	}

	activeSignal.multiChannelSignal(0).metrologySignal(Metrology::Channel_0);

	Metrology::Signal* pSignal = nullptr;

	if(theOptions.toolBar().signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
	{
		pSignal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).firstMetrologySignal();
	}
	else
	{
		pSignal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).firstMetrologySignal();
	}

	if (pSignal == nullptr)
	{
		return;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	int foundIndex = -1;

	int statisticCount = theSignalBase.statistic().count();
	for(int i = 0; i < statisticCount; i++)
	{
		if (theSignalBase.statistic().item(i).signal() == pSignal)
		{
			foundIndex = i;
			break;
		}
	}

	if (foundIndex == -1)
	{
		return;
	}

	m_pView->setCurrentIndex(m_pView->model()->index(foundIndex, 0));
}


// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateList()
{
	updateVisibleColunm();

	m_signalTable.clear();

	theSignalBase.statistic().updateStatistics();

	m_signalTable.set();

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateSignalInList(Hash signalHash)
{
	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	theSignalBase.statistic().updateStatistics(signalHash);

	m_signalTable.updateSignal(signalHash);

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateStatusBar()
{
	m_statusMeasureInavlid->setText(tr(" Invalid: %1").arg(theSignalBase.statistic().invalidMeasureCount()));
	m_statusMeasured->setText(tr(" Measured: %1 / %2").arg(theSignalBase.statistic().measuredCount()).arg(theSignalBase.statistic().count()));

	if (theSignalBase.statistic().invalidMeasureCount() == 0)
	{
		m_statusMeasureInavlid->hide();
	}
	else
	{
		m_statusMeasureInavlid->show();
		m_statusMeasureInavlid->setStyleSheet("background-color: rgb(255, 160, 160);");
	}
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateVisibleColunm()
{
	for(int c = 0; c < STATISTIC_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(STATISTIC_COLUMN_CUSTOM_ID, true);
	hideColumn(STATISTIC_COLUMN_EQUIPMENT_ID, true);
	hideColumn(STATISTIC_COLUMN_CMP_VALUE, m_measureType != MEASURE_TYPE_COMPARATOR);
	hideColumn(STATISTIC_COLUMN_CMP_NO, m_measureType != MEASURE_TYPE_COMPARATOR);
	hideColumn(STATISTIC_COLUMN_CMP_OUT_ID, true);
	hideColumn(STATISTIC_COLUMN_CHASSIS, true);
	hideColumn(STATISTIC_COLUMN_MODULE, true);
	hideColumn(STATISTIC_COLUMN_PLACE, true);
	hideColumn(STATISTIC_COLUMN_EL_RANGE, true);
	hideColumn(STATISTIC_COLUMN_EL_SENSOR, true);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::hideColumn(int column, bool hide)
{
	if (m_pView == nullptr)
	{
		return;
	}

	if (column < 0 || column >= STATISTIC_COLUMN_COUNT)
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

void StatisticPanel::exportSignal()
{
	ExportData* dialog = new ExportData(m_pView, false, "Statistics");
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::selectSignalForMeasure()
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return;
	}

	if (pMainWindow->rackCombo() == nullptr || pMainWindow->signalCombo() == nullptr)
	{
		return;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	int statisticItemIndex = m_pView->currentIndex().row();
	if (statisticItemIndex < 0 || statisticItemIndex >= theSignalBase.statistic().count())
	{
		return;
	}

	//
	//
	const StatisticItem& si = theSignalBase.statistic().item(statisticItemIndex);

	if (si.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNDEFINED)
	{
		if (si.signal() == nullptr || si.signal()->param().isValid() == false)
		{
			return;
		}

		QString str;

		str = tr("Signal %1 is \"%2\" signal.\nTo measure this signal you have to create connection with input signal.\nFor example, type of connection: \"Input\" -> \"%2\".\n\n"
				 "To create a new connection between signals, select \"View\"->\"Signal connections...\"")
				.arg(si.signal()->param().appSignalID())
				.arg(si.signal()->param().signalTypeStr());

		QMessageBox::information(this, windowTitle(), str);

		return;
	}

	//
	//

	Metrology::Signal* pSignal = nullptr;

	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:
			{
				pSignal = si.signal();
			}
			break;

		case MEASURE_TYPE_COMPARATOR:
			{
				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
				if (comparatorEx == nullptr)
				{
					break;
				}

				Metrology::Signal* pInputSignal = comparatorEx->inputSignal();
				if (pInputSignal == nullptr || pInputSignal->param().isValid() == false)
				{
					break;
				}

				if (pInputSignal->param().isInput() == true)
				{
					pSignal = pInputSignal;
					break;
				}

				int connectionIndex = theSignalBase.signalConnections().findIndex(MEASURE_IO_SIGNAL_TYPE_OUTPUT, pInputSignal);
				if (connectionIndex == -1)
				{
					break;
				}

				const SignalConnection& connection = theSignalBase.signalConnections().connection(connectionIndex);
				if (connection.isValid() == false)
				{
					break;
				}

				pSignal = connection.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);
			}
			break;

		default:
			assert(0);
			break;
	}

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	if (theOptions.toolBar().signalConnectionType() != si.signalConnectionType())
	{
		pMainWindow->signalConnectionTypeList()->setCurrentIndex(si.signalConnectionType());
	}

	//
	//
	int rackComboIndex = -1;

	int rackComboCount = pMainWindow->rackCombo()->count();
	for(int i = 0; i < rackComboCount; i++)
	{
		switch (theOptions.toolBar().measureKind())
		{
			case MEASURE_KIND_ONE_RACK:
			case MEASURE_KIND_ONE_MODULE:

				if (pMainWindow->rackCombo()->itemData(i).toInt() == pSignal->param().location().rack().index())
				{
					rackComboIndex = i;
				}

				break;

			case MEASURE_KIND_MULTI_RACK:

				if (pMainWindow->rackCombo()->itemData(i).toInt() == pSignal->param().location().rack().groupIndex())
				{
					rackComboIndex = i;
				}

				break;

			default:
				assert(0);
		}

		if (rackComboIndex != -1)
		{
			break;
		}
	}

	if (rackComboIndex == -1)
	{
		return;
	}

	//
	//
	pMainWindow->rackCombo()->setCurrentIndex(rackComboIndex);

	//
	//
	int measureSignalIndex = -1;

	int signalComboCount = theSignalBase.signalForMeasureCount();
	for(int i = 0; i < signalComboCount; i++)
	{
		const MeasureSignal& measureSignal = theSignalBase.signalForMeasure1(i);
		if (measureSignal.isEmpty() == true)
		{
			continue;
		}

		if (measureSignal.contains(pSignal) == false)
		{
			continue;
		}

		measureSignalIndex = i;

		break;
	}

	if (measureSignalIndex == -1)
	{
		return;
	}

	//
	//
	int signalComboIndex = -1;

	signalComboCount = pMainWindow->signalCombo()->count();
	for(int i = 0; i < signalComboCount; i++)
	{
		if (pMainWindow->signalCombo()->itemData(i).toInt() ==  measureSignalIndex)
		{
			signalComboIndex = i;
			break;
		}
	}

	if (signalComboIndex == -1)
	{
		return;
	}

	//
	//
	pMainWindow->signalCombo()->setCurrentIndex(signalComboIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::findSignalInMeasureList()
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return;
	}

	FindMeasurePanel* pFindMeasurePanel = pMainWindow->findMeasurePanel();
	if (pFindMeasurePanel == nullptr)
	{
		return;
	}

	int statisticItemIndex = m_pView->currentIndex().row();
	if (statisticItemIndex < 0 || statisticItemIndex >= theSignalBase.statistic().count())
	{
		return;
	}

	//
	//
	const StatisticItem& si = theSignalBase.statistic().item(statisticItemIndex);

	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	pFindMeasurePanel->show();
	pFindMeasurePanel->setFindText(pSignal->param().appSignalID());
	emit pFindMeasurePanel->find();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::find()
{
	if (m_pView == nullptr)
	{
		return;
	}

	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::copy()
{
	if (m_pView == nullptr)
	{
		return;
	}

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

void StatisticPanel::selectAll()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::onProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= theSignalBase.statistic().count())
	{
		return;
	}

	const StatisticItem& si = theSignalBase.statistic().item(index);

	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:
			{
				Metrology::Signal* pSignal = si.signal();
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					break;
				}

				SignalPropertyDialog dialog(pSignal->param());
				dialog.exec();
			}
			break;

		case MEASURE_TYPE_COMPARATOR:
			{
				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
				if (comparatorEx == nullptr)
				{
					break;
				}

				ComparatorPropertyDialog dialog(*comparatorEx);
				if (dialog.exec() != QDialog::Accepted)
				{
					break;
				}

				*comparatorEx = dialog.comparator();
			}
			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::gotoNextNotMeasured()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int statisticCount = theSignalBase.statistic().count();
	if (statisticCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < statisticCount; i++)
	{
		if (theSignalBase.statistic().item(i).isMeasured() == false)
		{
			foundIndex = i;

			break;
		}
	}

	if (foundIndex == -1)
	{
		return;
	}

	m_pView->setCurrentIndex(m_pView->model()->index(foundIndex, 0));
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::gotoNextInvalid()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int statisticCount = theSignalBase.statistic().count();
	if (statisticCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < statisticCount; i++)
	{
		if (theSignalBase.statistic().item(i).state() == StatisticItem::State::Failed)
		{
			foundIndex = i;

			break;
		}
	}

	if (foundIndex == -1)
	{
		return;
	}

	m_pView->setCurrentIndex(m_pView->model()->index(foundIndex, 0));
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	if (m_pFindSignalInMeasureList != nullptr)
	{
		int statisticItemIndex = m_pView->currentIndex().row();
		if (statisticItemIndex >= 0 && statisticItemIndex < theSignalBase.statistic().count())
		{
			const StatisticItem& si = theSignalBase.statistic().item(statisticItemIndex);

			m_pFindSignalInMeasureList->setEnabled(si.isMeasured());
		}
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_pColumnAction[STATISTIC_COLUMN_CMP_VALUE]->setDisabled(m_measureType != MEASURE_TYPE_COMPARATOR);
	m_pColumnAction[STATISTIC_COLUMN_CMP_NO]->setDisabled(m_measureType != MEASURE_TYPE_COMPARATOR);
	m_pColumnAction[STATISTIC_COLUMN_CMP_OUT_ID]->setDisabled(m_measureType != MEASURE_TYPE_COMPARATOR);

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
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
