#include "PanelStatistics.h"

#include "DialogMetrologyConnection.h"
#include "DialogObjectProperties.h"
#include "ProcessData.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticsTable::StatisticsTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

StatisticsTable::~StatisticsTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsTable::columnCount(const QModelIndex&) const
{
	return STATISTICS_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticsTable::rowCount(const QModelIndex&) const
{
	return m_statisticsItemCount;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticsTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < STATISTICS_COLUMN_COUNT)
		{
			result = qApp->translate("PanelStatistics", StatisticsColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant StatisticsTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= m_statisticsItemCount)
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > STATISTICS_COLUMN_COUNT)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case STATISTICS_COLUMN_APP_ID:				result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_CUSTOM_ID:			result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_EQUIPMENT_ID:		result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_CAPTION:				result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_CMP_VALUE:			result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_CMP_NO:				result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_CMP_OUT_ID:			result = Qt::AlignLeft;		break;
			case STATISTICS_COLUMN_RACK:				result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_CHASSIS:				result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_MODULE:				result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_PLACE:				result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_EL_RANGE:			result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_EL_SENSOR:			result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_EN_RANGE:			result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_SIGNAL_TYPE:			result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_SIGNAL_CONNECTION:	result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_MEASURE_COUNT:		result = Qt::AlignCenter;	break;
			case STATISTICS_COLUMN_STATE:				result = Qt::AlignCenter;	break;

			default:									assert(0);
		}

		return result;
	}

	const StatisticsItem& si = theSignalBase.statistics().item(row);

	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::ForegroundRole)
	{
		if (ERR_METROLOGY_CONNECTION_TYPE(si.connectionType()) == true)
		{
			if (column == STATISTICS_COLUMN_SIGNAL_TYPE || column == STATISTICS_COLUMN_SIGNAL_CONNECTION)
			{
				return QColor(Qt::red);
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
				case StatisticsItem::State::Failed:		return theOptions.measureView().colorErrorLimit();
				case StatisticsItem::State::Success:	return theOptions.measureView().colorNotError();
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

void StatisticsTable::set()
{
	m_statisticsItemCount = theSignalBase.statistics().count();
	if (m_statisticsItemCount == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, m_statisticsItemCount - 1);
	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsTable::clear()
{
	if (m_statisticsItemCount == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, m_statisticsItemCount - 1);
	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticsTable::text(int row, int column, const StatisticsItem& si) const
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

	if (theSignalBase.statistics().measureType() == Measure::Type::Comparators)
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

	if (row > 0 && theSignalBase.statistics().item(row - 1).signal() == pSignal)
	{
		visible = false;
	}

	QString result;

	switch (column)
	{
		case STATISTICS_COLUMN_APP_ID:				result = visible ? param.appSignalID() : QString();											break;
		case STATISTICS_COLUMN_CUSTOM_ID:			result = visible ? param.customAppSignalID() : QString();									break;
		case STATISTICS_COLUMN_EQUIPMENT_ID:		result = visible ? param.equipmentID() : QString();											break;
		case STATISTICS_COLUMN_CAPTION:				result = visible ? param.caption() : QString();												break;
		case STATISTICS_COLUMN_CMP_VALUE:			result = comparatorValue;																	break;
		case STATISTICS_COLUMN_CMP_NO:				result = comparatorNo;																		break;
		case STATISTICS_COLUMN_CMP_OUT_ID:			result = comparatorOutputID;																break;
		case STATISTICS_COLUMN_RACK:				result = visible ? param.location().rack().caption() : QString();							break;
		case STATISTICS_COLUMN_CHASSIS:				result = visible ? param.location().chassisStr() : QString();								break;
		case STATISTICS_COLUMN_MODULE:				result = visible ? param.location().moduleStr() : QString();								break;
		case STATISTICS_COLUMN_PLACE:				result = visible ? param.location().placeStr() : QString();									break;
		case STATISTICS_COLUMN_EL_RANGE:			result = param.electricRangeStr();															break;
		case STATISTICS_COLUMN_EL_SENSOR:			result = param.electricSensorTypeStr();														break;
		case STATISTICS_COLUMN_EN_RANGE:			result = param.engineeringRangeStr();														break;
		case STATISTICS_COLUMN_SIGNAL_TYPE:			result = qApp->translate("MetrologySignal", param.signalTypeStr().toUtf8());				break;
		case STATISTICS_COLUMN_SIGNAL_CONNECTION:	result = qApp->translate("StatisticsBase", si.connectionTypeStr().trimmed().toUtf8());		break;
		case STATISTICS_COLUMN_MEASURE_COUNT:		result = si.measureCountStr();																break;
		case STATISTICS_COLUMN_STATE:				result = qApp->translate("StatisticsBase", si.stateStr().toUtf8());							break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticsTable::updateSignal(Hash signalHash)
{
	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	std::vector<int> rowList;

	int count = theSignalBase.statistics().count();
	for(int i = 0; i < count; i++)
	{
		const StatisticsItem& si = theSignalBase.statistics().item(i);

		Metrology::Signal* pSignal = si.signal();
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		if (signalHash == pSignal->param().hash())
		{
			rowList.push_back(i);
		}
	}

	for(int row : rowList)
	{
		for (int column = 0; column < STATISTICS_COLUMN_COUNT; column ++)
		{
			QModelIndex cellIndex = index(row, column);

			emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Measure::Type PanelStatistics::m_measureType = Measure::Type::Linearity;
Measure::Kind PanelStatistics::m_measureKind = Measure::Kind::NoMeasureKind;
Metrology::ConnectionType PanelStatistics::m_connectionType = Metrology::ConnectionType::NoConnectionType;

// -------------------------------------------------------------------------------------------------------------------

PanelStatistics::PanelStatistics(QWidget* parent) :
	QDockWidget(parent)
{
	setWindowTitle(tr("Panel statistics (Checklist)"));
	setObjectName(windowTitle());

	createInterface();
	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

PanelStatistics::~PanelStatistics()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::createInterface()
{
	m_pStatisticsWindow = new QMainWindow;
	if (m_pStatisticsWindow == nullptr)
	{
		return;
	}

	m_pStatisticsWindow->installEventFilter(this);

	//
	//
	m_pMenuBar = new QMenuBar(m_pStatisticsWindow);
	m_pSignalMenu = new QMenu(tr("&Results"), m_pStatisticsWindow);
	m_pEditMenu = new QMenu(tr("&Edit"), m_pStatisticsWindow);
	m_pViewMenu = new QMenu(tr("&View"), m_pStatisticsWindow);

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

	m_pViewGotoMenu = new QMenu(tr("Go to next"), m_pStatisticsWindow);
	m_pGotoNextNotMeasuredAction = m_pViewGotoMenu->addAction(tr("Not measured"));
	m_pGotoNextInvalidAction = m_pViewGotoMenu->addAction(tr("Invalid"));

	m_pViewMenu->addMenu(m_pViewGotoMenu);
	m_pViewMenu->addSeparator();

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	connect(m_pExportAction, &QAction::triggered, this, &PanelStatistics::exportSignal);

	connect(m_pFindAction, &QAction::triggered, this, &PanelStatistics::find);
	connect(m_pCopyAction, &QAction::triggered, this, &PanelStatistics::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &PanelStatistics::selectAll);
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &PanelStatistics::onProperty);

	connect(m_pGotoNextNotMeasuredAction, &QAction::triggered, this, &PanelStatistics::gotoNextNotMeasured);
	connect(m_pGotoNextInvalidAction, &QAction::triggered, this, &PanelStatistics::gotoNextInvalid);

	m_pStatisticsWindow->setMenuBar(m_pMenuBar);

	//
	//
	m_pView = new QTableView(m_pStatisticsWindow);
	m_pView->setModel(&m_signalTable);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < STATISTICS_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, StatisticsColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &PanelStatistics::onListDoubleClicked);

	m_pStatisticsWindow->setCentralWidget(m_pView);

	setWidget(m_pStatisticsWindow);

	createStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::createHeaderContexMenu()
{
	if (m_pView == nullptr)
	{
		return;
	}

	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested,
			this, &PanelStatistics::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < STATISTICS_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(qApp->translate("PanelStatistics",
																				 StatisticsColumn[column]));
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);
		}
	}

	connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered),
			this, &PanelStatistics::onColumnAction);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::createContextMenu()
{
	if (m_pStatisticsWindow == nullptr)
	{
		return;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), m_pStatisticsWindow);

	m_pSelectSignalForMeasure = m_pContextMenu->addAction(tr("&Select signal for measuring"));
	m_pContextMenu->addSeparator();
	m_pFindSignalInStatisticsList = m_pContextMenu->addAction(tr("&Find signal in the statistics list ..."));
	m_pFindSignalInStatisticsList->setIcon(QIcon(":/icons/Find.png"));
	m_pFindSignalInMeasureList = m_pContextMenu->addAction(tr("&Find signal in the measure list ..."));
	m_pFindSignalInMeasureList->setIcon(QIcon(":/icons/Find.png"));
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSignalPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &PanelStatistics::onContextMenu);

	connect(m_pSelectSignalForMeasure, &QAction::triggered, this, &PanelStatistics::selectSignalForMeasure);
	connect(m_pFindSignalInStatisticsList, &QAction::triggered, this, &PanelStatistics::findSignalInStatisticsList);
	connect(m_pFindSignalInMeasureList, &QAction::triggered, this, &PanelStatistics::findSignalInMeasureList);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::createStatusBar()
{
	if (m_pStatisticsWindow == nullptr)
	{
		return;
	}

	m_pStatusBar = m_pStatisticsWindow->statusBar();

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

bool PanelStatistics::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent* >(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			selectSignalForMeasure();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::setViewFont(const QFont& font)
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->setFont(font);
	QSize cellSize = QFontMetrics(font).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::measureTypeChanged(Measure::Type measureType)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	m_measureType = measureType;

	theSignalBase.statistics().setMeasureType(m_measureType);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::measureKindChanged(Measure::Kind measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = measureKind;
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::connectionTypeChanged(Metrology::ConnectionType connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	m_connectionType = connectionType;

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::activeSignalChanged(const MeasureSignal& activeSignal)
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

	Metrology::Signal* pSignal = nullptr;

	if(m_connectionType == Metrology::ConnectionType::Unused)
	{
		pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).firstMetrologySignal();
	}
	else
	{
		pSignal = activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination).firstMetrologySignal();
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

	int statisticCount = theSignalBase.statistics().count();
	for(int i = 0; i < statisticCount; i++)
	{
		if (theSignalBase.statistics().item(i).signal() == pSignal)
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

void PanelStatistics::updateList()
{
	if (m_pMeasureBase == nullptr)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	updateVisibleColunm();

	m_signalTable.clear();

		m_pMeasureBase->updateStatisticsBase(m_measureType);

	m_signalTable.set();

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::updateSignalInList(Hash signalHash)
{
	if (m_pMeasureBase == nullptr)
	{
		return;
	}

	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		return;
	}

	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	m_pMeasureBase->updateStatisticsBase(m_measureType, signalHash);

	m_signalTable.updateSignal(signalHash);

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::updateStatusBar()
{
	m_statusMeasureInavlid->setText(tr(" Invalid: %1").
									arg(theSignalBase.statistics().
										invalidMeasureCount()));

	m_statusMeasured->setText(tr(" Measured: %1 / %2").
							  arg(theSignalBase.statistics().measuredCount()).
							  arg(theSignalBase.statistics().count()));

	if (theSignalBase.statistics().invalidMeasureCount() == 0)
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

void PanelStatistics::updateVisibleColunm()
{
	for(int c = 0; c < STATISTICS_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(STATISTICS_COLUMN_CUSTOM_ID, true);
	hideColumn(STATISTICS_COLUMN_EQUIPMENT_ID, true);
	hideColumn(STATISTICS_COLUMN_CMP_VALUE, m_measureType != Measure::Type::Comparators);
	hideColumn(STATISTICS_COLUMN_CMP_NO, m_measureType != Measure::Type::Comparators);
	hideColumn(STATISTICS_COLUMN_CMP_OUT_ID, true);
	hideColumn(STATISTICS_COLUMN_CHASSIS, true);
	hideColumn(STATISTICS_COLUMN_MODULE, true);
	hideColumn(STATISTICS_COLUMN_PLACE, true);
	hideColumn(STATISTICS_COLUMN_EL_RANGE, true);
	hideColumn(STATISTICS_COLUMN_EL_SENSOR, true);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::hideColumn(int column, bool hide)
{
	if (m_pView == nullptr)
	{
		return;
	}

	if (column < 0 || column >= STATISTICS_COLUMN_COUNT)
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

void PanelStatistics::exportSignal()
{
	ExportData* dialog = new ExportData(m_pView, false, "Statistics");
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::selectSignalForMeasure()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int statisticItemIndex = m_pView->currentIndex().row();
	if (statisticItemIndex < 0 || statisticItemIndex >= theSignalBase.statistics().count())
	{
		return;
	}

	//
	//
	const StatisticsItem& si = theSignalBase.statistics().item(statisticItemIndex);

	if (ERR_METROLOGY_CONNECTION_TYPE(si.connectionType()) == true)
	{
		if (si.signal() == nullptr || si.signal()->param().isValid() == false)
		{
			return;
		}

		QString str;

		str = tr("Signal %1 is \"%2\" signal.\n"
				 "To measure this signal you have to create connection with input signal.\n"
				 "For example, type of connection: \"Input\" -> \"%2\".\n\n"
				 "To create a new connection between signals, select \"View\"->\"Metrology connections...\"\n\n"
				 "Do you want to create new connection now?")
				.arg(si.signal()->param().appSignalID())
				.arg(qApp->translate("MetrologySignal", si.signal()->param().signalTypeStr().toUtf8()));

		int result = QMessageBox::question(this, windowTitle(), str);
		if (result == QMessageBox::No)
		{
			return;
		}

		DialogMetrologyConnection dialog(si.signal(), this);
		if (dialog.exec() != QDialog::Accepted)
		{
			return;
		}

		// if save in local file
		//
//		if (theSignalBase.metrologyConnections().save() == false)
//		{
//			QMessageBox::information(this,
//									 windowTitle(),
//									 tr("Attempt to save metrology connections was unsuccessfully!"));
//			return;
//		}

		theSignalBase.statistics().createSignalList();
		theSignalBase.statistics().createComparatorList();

		updateList();

		return;
	}

	//
	//

	Metrology::Signal* pSignal = nullptr;

	switch (m_measureType)
	{
		case Measure::Type::Linearity:
			{
				pSignal = si.signal();
			}
			break;

		case Measure::Type::Comparators:
			{
				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
				if (comparatorEx == nullptr)
				{
					break;
				}

				pSignal = comparatorEx->inputSignal();
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

	if (pSignal->param().isInput() == false)
	{
		int connectionIndex = theSignalBase.connections().findConnectionIndex(Metrology::ConnectionIoType::Destination, pSignal);
		if (connectionIndex == -1)
		{
			return;
		}

		const Metrology::Connection& connection = theSignalBase.connections().connection(connectionIndex);
		if (connection.isValid() == false)
		{
			return;
		}

		pSignal = connection.metrologySignal(Metrology::ConnectionIoType::Source);
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			return;
		}
	}

	// set ConnectionType in main window
	//
	if (m_connectionType != si.connectionType())
	{
		emit setConnectionType(si.connectionType());
	}

	// find Rack
	//
	int rackIndex = -1;

	int rackCount = theSignalBase.rackForMeasureCount();
	for(int i = 0; i < rackCount; i++)
	{
		const Metrology::RackParam& rack = theSignalBase.rackForMeasure(i);
		if (rack.isValid() == false)
		{
			continue;
		}

		switch (m_measureKind)
		{
			case Measure::Kind::OneRack:
			case Measure::Kind::OneModule:

				if (rack.index() != pSignal->param().location().rack().index())
				{
					continue;
				}

				rackIndex = i;
				break;

			case Measure::Kind::MultiRack:

				if (rack.index() != pSignal->param().location().rack().groupIndex())
				{
					continue;
				}

				rackIndex = i;
				break;

			default:
				assert(0);
		}

		if (rackIndex != -1)
		{
			break;
		}
	}

	if (rackIndex == -1)
	{
		return;
	}

	// set Rack
	//
	emit setRack(rackIndex);

	// find Signal
	//
	int measureSignalIndex = -1;

	int measureSignalCount = theSignalBase.signalForMeasureCount();
	for(int i = 0; i < measureSignalCount; i++)
	{
		const MeasureSignal& measureSignal = theSignalBase.signalForMeasure(i);
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

	emit setMeasureSignal(measureSignalIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::findSignalInStatisticsList()
{
	find();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::findSignalInMeasureList()
{
	int statisticItemIndex = m_pView->currentIndex().row();
	if (statisticItemIndex < 0 || statisticItemIndex >= theSignalBase.statistics().count())
	{
		return;
	}

	//
	//
	const StatisticsItem& si = theSignalBase.statistics().item(statisticItemIndex);

	Metrology::Signal* pSignal = si.signal();
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return;
	}

	emit showFindMeasurePanel(pSignal->param().appSignalID());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::find()
{
	if (m_pView == nullptr)
	{
		return;
	}

	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::selectAll()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->selectAll();
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::onProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= theSignalBase.statistics().count())
	{
		return;
	}

	const StatisticsItem& si = theSignalBase.statistics().item(index);

	switch (m_measureType)
	{
		case Measure::Type::Linearity:
			{
				Metrology::Signal* pSignal = si.signal();
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					break;
				}

				DialogSignalProperty dialog(pSignal->param(), this);
				dialog.exec();
			}
			break;

		case Measure::Type::Comparators:
			{
				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = si.comparator();
				if (comparatorEx == nullptr)
				{
					break;
				}

				DialogComparatorProperty dialog(*comparatorEx, this);
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

void PanelStatistics::gotoNextNotMeasured()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int statisticCount = theSignalBase.statistics().count();
	if (statisticCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < statisticCount; i++)
	{
		if (theSignalBase.statistics().item(i).isMeasured() == false)
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

void PanelStatistics::gotoNextInvalid()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int statisticCount = theSignalBase.statistics().count();
	if (statisticCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < statisticCount; i++)
	{
		if (theSignalBase.statistics().item(i).state() == StatisticsItem::State::Failed)
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

void PanelStatistics::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	if (m_pFindSignalInMeasureList != nullptr)
	{
		int statisticItemIndex = m_pView->currentIndex().row();
		if (statisticItemIndex >= 0 && statisticItemIndex < theSignalBase.statistics().count())
		{
			const StatisticsItem& si = theSignalBase.statistics().item(statisticItemIndex);

			m_pFindSignalInMeasureList->setEnabled(si.isMeasured());
		}
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_pColumnAction[STATISTICS_COLUMN_CMP_VALUE]->setDisabled(m_measureType != Measure::Type::Comparators);
	m_pColumnAction[STATISTICS_COLUMN_CMP_NO]->setDisabled(m_measureType != Measure::Type::Comparators);
	m_pColumnAction[STATISTICS_COLUMN_CMP_OUT_ID]->setDisabled(m_measureType != Measure::Type::Comparators);

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void PanelStatistics::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < STATISTICS_COLUMN_COUNT; column++)
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
