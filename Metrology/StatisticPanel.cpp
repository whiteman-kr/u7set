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
	return theSignalBase.statistic().signalCount();
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
			result = StatisticColumn[section];
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
	if (row < 0 || row >= theSignalBase.statistic().signalCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > STATISTIC_COLUMN_COUNT)
	{
		return QVariant();
	}

	Metrology::Signal* pSignal = theSignalBase.statistic().signal(row);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case STATISTIC_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_EQUIPMENT_ID:		result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CAPTION:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_RACK:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_CHASSIS:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MODULE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_PLACE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_ADC:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_EL_RANGE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_PH_RANGE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_EN_RANGE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MEASURE_COUNT:	result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_STATE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_OUTPUT_TYPE:		result = Qt::AlignCenter;	break;
			default:								assert(0);
		}

		return result;
	}

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
	}

	if (role == Qt::TextColorRole)
	{
		switch (theOptions.toolBar().outputSignalType())
		{
			case OUTPUT_SIGNAL_TYPE_UNUSED:

				if (pSignal->param().isOutput() == true)
				{
					return QColor(Qt::lightGray);
				}

				break;

			case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
			case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

				if (pSignal->param().isInput() == true)
				{
					return QColor(Qt::lightGray);
				}

				break;

			default:
				break;
		}

		if (column == STATISTIC_COLUMN_STATE && pSignal->statistic().measureCount() == 0)
		{
			return QColor(Qt::lightGray);
		}

		return QVariant();
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (column == STATISTIC_COLUMN_CUSTOM_ID || column == STATISTIC_COLUMN_EQUIPMENT_ID || column == STATISTIC_COLUMN_STATE)
		{
			if (pSignal->statistic().measureCount() != 0)
			{
				switch (pSignal->statistic().state())
				{
					case Metrology::StatisticStateFailed:	return theOptions.measureView().colorErrorLimit();	break;
					case Metrology::StatisticStateSuccess:	return theOptions.measureView().colorNotError();	break;
				}
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignal);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::set()
{
	int count = theSignalBase.statistic().signalCount();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);
	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::clear()
{
	int count = theSignalBase.statistic().signalCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);
	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticTable::text(int row, int column, Metrology::Signal* pSignal) const
{
	if (row < 0 || row >= theSignalBase.statistic().signalCount())
	{
		return QString();
	}

	if (column < 0 || column > STATISTIC_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSignal == nullptr)
	{
		return QString();
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case STATISTIC_COLUMN_APP_ID:			result = param.appSignalID();						break;
		case STATISTIC_COLUMN_CUSTOM_ID:		result = param.customAppSignalID();					break;
		case STATISTIC_COLUMN_EQUIPMENT_ID:		result = param.location().equipmentID();			break;
		case STATISTIC_COLUMN_CAPTION:			result = param.caption();							break;
		case STATISTIC_COLUMN_RACK:				result = param.location().rack().caption();			break;
		case STATISTIC_COLUMN_CHASSIS:			result = param.location().chassisStr();				break;
		case STATISTIC_COLUMN_MODULE:			result = param.location().moduleStr();				break;
		case STATISTIC_COLUMN_PLACE:			result = param.location().placeStr();				break;
		case STATISTIC_COLUMN_ADC:				result = param.adcRangeStr(true);					break;
		case STATISTIC_COLUMN_EL_RANGE:			result = param.electricRangeStr();					break;
		case STATISTIC_COLUMN_PH_RANGE:			result = param.physicalRangeStr();					break;
		case STATISTIC_COLUMN_EN_RANGE:			result = param.engeneeringRangeStr();				break;
		case STATISTIC_COLUMN_MEASURE_COUNT:	result = pSignal->statistic().measureCountStr();	break;
		case STATISTIC_COLUMN_STATE:			result = pSignal->statistic().stateStr();			break;
		case STATISTIC_COLUMN_OUTPUT_TYPE:		result.clear();										break;
		default:								assert(0);
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

	int row = -1;

	int count = theSignalBase.statistic().signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.statistic().signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->param().isValid() == false)
		{
			continue;
		}

		if (signalHash == pSignal->param().hash())
		{
			row = i;
			break;
		}
	}

	if (row == -1)
	{
		return;
	}

	for (int column = 0; column < STATISTIC_COLUMN_COUNT; column ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
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

	setWindowTitle("Panel statistics");
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
	m_pSignalMenu = new QMenu(tr("&Signal"), m_pStatisticWindow);
	m_pEditMenu = new QMenu(tr("&Edit"), m_pStatisticWindow);
	m_pViewMenu = new QMenu(tr("&View"), m_pStatisticWindow);

	m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));

	m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));

	m_pEditMenu->addSeparator();

	m_pSignalPropertyAction = m_pEditMenu->addAction(tr("Properties ..."));
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
	connect(m_pSignalPropertyAction, &QAction::triggered, this, &StatisticPanel::signalProperty);

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

	connect(m_pView, &QTableView::doubleClicked , this, &StatisticPanel::onListDoubleClicked);


	StatisticsStateDelegate* stateDelegate = new StatisticsStateDelegate(m_pStatisticWindow);
	m_pView->setItemDelegateForColumn(STATISTIC_COLUMN_APP_ID, stateDelegate);


	m_pStatisticWindow->setCentralWidget(m_pView);

	//
	//
	createStatusBar();

	//
	//
	setWidget(m_pStatisticWindow);
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
		m_pColumnAction[column] = m_headerContextMenu->addAction(StatisticColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &StatisticPanel::onColumnAction);
		}
	}
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
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pSignalPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &StatisticPanel::onContextMenu);

	connect(m_pSelectSignalForMeasure, &QAction::triggered, this, &StatisticPanel::selectSignalForMeasure);
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

	m_pStatusBar->addWidget(m_statusMeasureInavlid);
	m_pStatusBar->addWidget(m_statusMeasured);
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

		if (keyEvent->key() == Qt::Key_Return)
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

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::changedOutputSignalType(int type)
{
	if (type < 0 || type >= OUTPUT_SIGNAL_TYPE_COUNT)
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

	activeSignal.multiSignal(0).metrologySignal(Metrology::Channel_0);

	Metrology::Signal* pSignal = nullptr;

	switch (theOptions.toolBar().outputSignalType())
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:			pSignal = activeSignal.multiSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).firstMetrologySignal();	break;
		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:	pSignal = activeSignal.multiSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).firstMetrologySignal();	break;
		default:								assert(0);																					break;
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

	int signaCount = theSignalBase.statistic().signalCount();
	for(int i = 0; i < signaCount; i++)
	{
		if (theSignalBase.statistic().signal(i) == pSignal)
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

	// temporary solution
	//
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return;
	}

	MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
	if (pMeasureView == nullptr)
	{
		return;
	}

	m_signalTable.clear();

	theSignalBase.statistic().updateSignalsState(pMeasureView);

	m_signalTable.set();
	//
	// temporary solution

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateSignalInList(Hash signalHash)
{
	if (signalHash == UNDEFINED_HASH)
	{
		return;
	}

	// temporary solution
	//
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return;
	}

	MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
	if (pMeasureView == nullptr)
	{
		return;
	}

	theSignalBase.statistic().updateSignalState(pMeasureView, signalHash);

	m_signalTable.updateSignal(signalHash);
	//
	// temporary solution

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::updateStatusBar()
{
	m_statusMeasured->setText(tr(" Measured: %1 / %2").arg(theSignalBase.statistic().measuredCount()).arg(theSignalBase.statistic().signalCount()));
	m_statusMeasureInavlid->setText(tr(" Invalid: %1").arg(theSignalBase.statistic().invalidMeasureCount()));

	if (theSignalBase.statistic().invalidMeasureCount() == 0)
	{
		m_statusMeasureInavlid->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	}
	else
	{
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
	hideColumn(STATISTIC_COLUMN_ADC, true);
	hideColumn(STATISTIC_COLUMN_PH_RANGE, true);
	hideColumn(STATISTIC_COLUMN_EL_RANGE, true);
	hideColumn(STATISTIC_COLUMN_OUTPUT_TYPE, true);
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
	ExportData* dialog = new ExportData(m_pView, tr("Statistics"));
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

	//
	//
	int metrologySignalIndex = m_pView->currentIndex().row();

	Metrology::Signal* pMetrologySignal = theSignalBase.statistic().signal(metrologySignalIndex);
	if (pMetrologySignal == nullptr || pMetrologySignal->param().isValid() == false)
	{
		return;
	}

	switch (theOptions.toolBar().outputSignalType())
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:

			if (pMetrologySignal->param().isOutput() == true)
			{
				QMessageBox::information(this, windowTitle(), tr("Choose another type of signal"));
				return;
			}

			break;

		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

			if (pMetrologySignal->param().isInput() == true)
			{
				QMessageBox::information(this, windowTitle(), tr("Choose another type of signal"));
				return;
			}

			break;

		default:
			assert(0);
			break;
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

				if (pMainWindow->rackCombo()->itemData(i).toInt() == pMetrologySignal->param().location().rack().index())
				{
					rackComboIndex = i;
				}

				break;

			case MEASURE_KIND_MULTI_RACK:

				if (pMainWindow->rackCombo()->itemData(i).toInt() == pMetrologySignal->param().location().rack().groupIndex())
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
		MeasureSignal measureSignal = theSignalBase.signalForMeasure(i);
		if (measureSignal.isEmpty() == true)
		{
			continue;
		}

		if (measureSignal.contains(pMetrologySignal) == false)
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

void StatisticPanel::signalProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= theSignalBase.statistic().signalCount())
	{
		return;
	}

	Metrology::Signal* pSignal = theSignalBase.statistic().signal(index);
	if (pSignal == nullptr)
	{
		return;
	}

	if (pSignal->param().isValid() == false)
	{
		return;
	}

	SignalPropertyDialog dialog(pSignal->param());
	dialog.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::gotoNextNotMeasured()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int signaCount = theSignalBase.statistic().signalCount();
	if (signaCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < signaCount; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.statistic().signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->statistic().measureCount() == 0)
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

	int signaCount = theSignalBase.statistic().signalCount();
	if (signaCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < signaCount; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.statistic().signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->statistic().state() == Metrology::StatisticStateFailed)
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
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticPanel::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

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
