#include "Statistic.h"

#include <QClipboard>
#include <QHeaderView>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool StatisticTable::m_showADCInHex = true;

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::StatisticTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

StatisticTable::~StatisticTable()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::columnCount(const QModelIndex&) const
{
	return STATISTIC_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::rowCount(const QModelIndex&) const
{
	return signalCount();
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
	if (row < 0 || row >= signalCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > STATISTIC_COLUMN_COUNT)
	{
		return QVariant();
	}

	Metrology::Signal* pSignal = signal(row);
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case STATISTIC_COLUMN_RACK:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_APP_ID:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CUSTOM_ID:		result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_EQUIPMENT_ID:		result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CAPTION:			result = Qt::AlignLeft;		break;
			case STATISTIC_COLUMN_CHASSIS:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MODULE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_PLACE:			result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_ADC:				result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_IN_PH_RANGE:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_IN_EL_RANGE:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_OUTPUT_TYPE:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_OUT_PH_RANGE:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_OUT_EL_RANGE:		result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_MEASURE_COUNT:	result = Qt::AlignCenter;	break;
			case STATISTIC_COLUMN_STATE:			result = Qt::AlignCenter;	break;
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
		if (column == STATISTIC_COLUMN_STATE && pSignal->statistic().measureCount() == 0)
		{
			return QColor(Qt::lightGray);
		}

		return QVariant();
	}

	if (role == Qt::BackgroundColorRole)
	{
		if (column == STATISTIC_COLUMN_STATE && pSignal->statistic().measureCount() != 0)
		{
			if (pSignal->statistic().state() == Metrology::StatisticStateInvalid)
			{
				return theOptions.measureView().colorErrorLimit();
			}
			if (pSignal->statistic().state() == Metrology::StatisticStateSuccess)
			{
				return theOptions.measureView().colorNotError();
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

QString StatisticTable::text(int row, int column, Metrology::Signal* pSignal) const
{
	if (row < 0 || row >= signalCount())
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
		case STATISTIC_COLUMN_RACK:				result = param.location().rack().caption();			break;
		case STATISTIC_COLUMN_APP_ID:			result = param.appSignalID();						break;
		case STATISTIC_COLUMN_CUSTOM_ID:		result = param.customAppSignalID();					break;
		case STATISTIC_COLUMN_EQUIPMENT_ID:		result = param.location().equipmentID();			break;
		case STATISTIC_COLUMN_CAPTION:			result = param.caption();							break;
		case STATISTIC_COLUMN_CHASSIS:			result = param.location().chassisStr();				break;
		case STATISTIC_COLUMN_MODULE:			result = param.location().moduleStr();				break;
		case STATISTIC_COLUMN_PLACE:			result = param.location().placeStr();				break;
		case STATISTIC_COLUMN_ADC:				result = param.adcRangeStr(m_showADCInHex);			break;
		case STATISTIC_COLUMN_IN_PH_RANGE:		result = param.inputPhysicalRangeStr();				break;
		case STATISTIC_COLUMN_IN_EL_RANGE:		result = param.inputElectricRangeStr();				break;
		case STATISTIC_COLUMN_OUTPUT_TYPE:		result.clear();										break;
		case STATISTIC_COLUMN_OUT_PH_RANGE:		result = param.outputPhysicalRangeStr();			break;
		case STATISTIC_COLUMN_OUT_EL_RANGE:		result = param.outputElectricRangeStr();			break;
		case STATISTIC_COLUMN_MEASURE_COUNT:	result = pSignal->statistic().measureCountStr();	break;
		case STATISTIC_COLUMN_STATE:			result = pSignal->statistic().stateStr();			break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int StatisticTable::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* StatisticTable::signal(int index) const
{
	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			 pSignal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::set(const QList<Metrology::Signal*> list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList = list_add;

		m_signalMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticTable::clear()
{
	int count = signalCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_signalMutex.lock();

			m_signalList.clear();

		m_signalMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int StatisticDialog::m_measureType = MEASURE_TYPE_LINEARITY;

// -------------------------------------------------------------------------------------------------------------------

StatisticDialog::StatisticDialog(QWidget *parent) :
	QDialog(parent)
{
	m_pMainWindow = dynamic_cast<QMainWindow*> (parent);

	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->configSocket() != nullptr)
	{
		m_measureType = pMainWindow->measureType();

		connect(pMainWindow->configSocket(), &ConfigSocket::configurationLoaded, this, &StatisticDialog::updateList, Qt::QueuedConnection);
		connect(&pMainWindow->measureThread(), &MeasureThread::measureComplite, this, &StatisticDialog::updateList, Qt::QueuedConnection);
	}

	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

StatisticDialog::~StatisticDialog()
{
	m_signalTable.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Statistics.png"));
	setWindowTitle(tr("Statistics"));
	resize(QApplication::desktop()->availableGeometry().width() - 800, 800);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	m_pMenuBar = new QMenuBar(this);
	m_pSignalMenu = new QMenu(tr("&Signal"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);
	m_pViewMenu = new QMenu(tr("&View"), this);

	m_pExportAction = m_pSignalMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
	m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pEditMenu->addSeparator();

	m_pViewMeasureTypeMenu = new QMenu(tr("Measure type"), this);
	m_pTypeLinearityAction = m_pViewMeasureTypeMenu->addAction(tr("Linearity"));
	m_pTypeLinearityAction->setCheckable(true);
	m_pTypeLinearityAction->setChecked(m_measureType == MEASURE_TYPE_LINEARITY);

	m_pTypeComparatorsAction = m_pViewMeasureTypeMenu->addAction(tr("Comparators"));
	m_pTypeComparatorsAction->setCheckable(true);
	m_pTypeComparatorsAction->setChecked(m_measureType == MEASURE_TYPE_COMPARATOR);

	m_pViewShowMenu = new QMenu(tr("Show"), this);

	m_pShowADCInHexAction = m_pViewShowMenu->addAction(tr("ADC in Hex"));
	m_pShowADCInHexAction->setCheckable(true);
	m_pShowADCInHexAction->setChecked(m_signalTable.showADCInHex());

	m_pViewGotoMenu = new QMenu(tr("Go to next"), this);
	m_pGotoNextNotMeasuredAction = m_pViewGotoMenu->addAction(tr("Not measured"));
	m_pGotoNextInvalidAction = m_pViewGotoMenu->addAction(tr("Invalid"));

	m_pViewMenu->addMenu(m_pViewMeasureTypeMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addMenu(m_pViewShowMenu);
	m_pViewMenu->addSeparator();
	m_pViewMenu->addMenu(m_pViewGotoMenu);

	m_pMenuBar->addMenu(m_pSignalMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pViewMenu);

	createStatusBar();

	connect(m_pExportAction, &QAction::triggered, this, &StatisticDialog::exportSignal);

	connect(m_pFindAction, &QAction::triggered, this, &StatisticDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &StatisticDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &StatisticDialog::selectAll);

	connect(m_pTypeLinearityAction, &QAction::triggered, this, &StatisticDialog::showTypeLinearity);
	connect(m_pTypeComparatorsAction, &QAction::triggered, this, &StatisticDialog::showTypeComparators);
	connect(m_pShowADCInHexAction, &QAction::triggered, this, &StatisticDialog::showADCInHex);
	connect(m_pGotoNextNotMeasuredAction, &QAction::triggered, this, &StatisticDialog::gotoNextNotMeasured);
	connect(m_pGotoNextInvalidAction, &QAction::triggered, this, &StatisticDialog::gotoNextInvalid);


	m_pView = new QTableView(this);
	m_pView->setModel(&m_signalTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, StatisticColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);


	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_pStatusBar);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	setLayout(mainLayout);

	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &StatisticDialog::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < STATISTIC_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(StatisticColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &StatisticDialog::onColumnAction);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pSelectSignalForMeasure = m_pContextMenu->addAction(tr("&Select signal for measuring"));
	m_pSelectSignalForMeasure->setIcon(QIcon(":/icons/Start.png"));

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &StatisticDialog::onContextMenu);

	connect(m_pSelectSignalForMeasure, &QAction::triggered, this, &StatisticDialog::selectSignalForMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::createStatusBar()
{
	m_pStatusBar = new QStatusBar(this);

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

void StatisticDialog::updateList()
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_pMainWindow);
	if (pMainWindow == nullptr)
	{
		return ;
	}

	m_MeasuredCount = 0;
	m_invalidMeasureCount = 0;

	updateVisibleColunm();

	m_signalTable.clear();

	QList<Metrology::Signal*> signalList;

	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.isAnalog() == false || param.isInput() == false)
		{
			continue;
		}

		if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
		{
			continue;
		}

		// temporary solution // signal.setStatistic(theMeasureBase.statisticItem(param.hash()));
		//
			MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
			if (pMeasureView != nullptr)
			{
				pSignal->setStatistic(pMeasureView->table().m_measureBase.statistic(param.hash()));
			}
		//
		// temporary solution

		if (pSignal->statistic().measureCount() != 0)
		{
			m_MeasuredCount++;
		}

		if (pSignal->statistic().state() == Metrology::StatisticStateInvalid)
		{
			m_invalidMeasureCount ++;
		}

		signalList.append(pSignal);
	}

	m_signalTable.set(signalList);

	updateStatusBar();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateStatusBar()
{
	int signalCount = m_signalTable.signalCount();

	m_statusMeasured->setText(tr(" Measured: %1 / %2").arg(m_MeasuredCount).arg(signalCount));
	m_statusMeasureInavlid->setText(tr(" Invalid: %1").arg(m_invalidMeasureCount));

	if (m_invalidMeasureCount == 0)
	{
		m_statusMeasureInavlid->setStyleSheet("background-color: rgb(0xFF, 0xFF, 0xFF);");
	}
	else
	{
		m_statusMeasureInavlid->setStyleSheet("background-color: rgb(255, 160, 160);");
	}
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::updateVisibleColunm()
{
	for(int c = 0; c < STATISTIC_COLUMN_COUNT; c++)
	{
		hideColumn(c, false);
	}

	hideColumn(STATISTIC_COLUMN_CUSTOM_ID, true);
	hideColumn(STATISTIC_COLUMN_EQUIPMENT_ID, true);
	hideColumn(STATISTIC_COLUMN_ADC, true);
	hideColumn(STATISTIC_COLUMN_IN_PH_RANGE, true);
	hideColumn(STATISTIC_COLUMN_IN_EL_RANGE, true);
	hideColumn(STATISTIC_COLUMN_OUTPUT_TYPE, true);
	hideColumn(STATISTIC_COLUMN_OUT_PH_RANGE, true);
	hideColumn(STATISTIC_COLUMN_OUT_EL_RANGE, true);
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::hideColumn(int column, bool hide)
{
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

void StatisticDialog::exportSignal()
{
	ExportData* dialog = new ExportData(m_pView, tr("Statistics"));
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::selectSignalForMeasure()
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

	//
	//
	int metrologySignalIndex = m_pView->currentIndex().row();
	if (metrologySignalIndex < 0 || metrologySignalIndex >= m_signalTable.signalCount())
	{
		return;
	}

	Metrology::Signal* pMetrologySignal = m_signalTable.signal(metrologySignalIndex);
	if (pMetrologySignal == nullptr || pMetrologySignal->param().isValid() == false)
	{
		return;
	}

	//
	//
	int rackComboIndex = -1;

	int rackComboCount = pMainWindow->rackCombo()->count();
	for(int i = 0; i < rackComboCount; i++)
	{
		switch (theOptions.toolBar().measureKind())
		{
			case MEASURE_KIND_ONE:

				if (pMainWindow->rackCombo()->itemData(i).toInt() == pMetrologySignal->param().location().rack().index())
				{
					rackComboIndex = i;
				}

				break;

			case MEASURE_KIND_MULTI:

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

void StatisticDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::copy()
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

void StatisticDialog::showTypeLinearity()
{
	m_measureType = MEASURE_TYPE_LINEARITY;

	m_pTypeLinearityAction->setChecked(true);
	m_pTypeComparatorsAction->setChecked(false);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showTypeComparators()
{
	m_measureType = MEASURE_TYPE_COMPARATOR;

	m_pTypeLinearityAction->setChecked(false);
	m_pTypeComparatorsAction->setChecked(true);

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::showADCInHex()
{
	m_signalTable.setShowADCInHex(m_pShowADCInHexAction->isChecked());

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::gotoNextNotMeasured()
{
	int signaCount = m_signalTable.signalCount();
	if (signaCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < signaCount; i++)
	{
		Metrology::Signal* pSignal = m_signalTable.signal(i);
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

void StatisticDialog::gotoNextInvalid()
{
	int signaCount = m_signalTable.signalCount();
	if (signaCount == 0)
	{
		return;
	}

	int startIndex = m_pView->currentIndex().row() ;
	int foundIndex = -1;

	for(int i = startIndex + 1; i < signaCount; i++)
	{
		Metrology::Signal* pSignal = m_signalTable.signal(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->param().isValid() == false)
		{
			continue;
		}

		if (pSignal->statistic().state() == Metrology::StatisticStateInvalid)
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

void StatisticDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void StatisticDialog::onColumnAction(QAction* action)
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
