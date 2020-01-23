#include "History.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog(const SignalForLog& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::SignalForLog(PS::Signal* pSignal, double prevState, double state) :
	m_pSignal(pSignal),
	m_prevState(prevState),
	m_state(state)
{
	QDateTime cdt = QDateTime::currentDateTime();

	m_time.sprintf("%02d-%02d-%04d %02d:%02d:%02d:%03d",
					cdt.date().day(),
					cdt.date().month(),
					cdt.date().year(),

					cdt.time().hour(),
					cdt.time().minute(),
					cdt.time().second(),
					cdt.time().msec());
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog::~SignalForLog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalForLog::clear()
{
	m_time.clear();

	m_pSignal = nullptr;

	m_prevState = 0.0;
	m_state = 0.0;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalForLog::stateStr(double state) const
{
	if (m_pSignal == nullptr)
	{
		return QString();
	}

	QString str, formatStr;

	switch (m_pSignal->signalType())
	{
		case E::SignalType::Analog:

			switch (m_pSignal->analogSignalFormat())
			{
				case E::AnalogAppSignalFormat::SignedInt32:		formatStr.sprintf("%%.%df", 0);								break;
				case E::AnalogAppSignalFormat::Float32:			formatStr.sprintf("%%.%df", m_pSignal->decimalPlaces());	break;
				default:										assert(0);													break;
			}

			str.sprintf(formatStr.toLocal8Bit(), state);

			if (m_pSignal->unit().isEmpty() == false)
			{
				str.append(" " + m_pSignal->unit());
			}

			break;

		case E::SignalType::Discrete:

			str = state == 0.0 ? "No (0)" : "Yes (1)";

			break;
	}

	return str;
}


// -------------------------------------------------------------------------------------------------------------------

SignalForLog& SignalForLog::operator=(const SignalForLog& from)
{
	m_signalMutex.lock();

		m_time = from.m_time;

		m_pSignal = from.m_pSignal;

		m_prevState = from.m_prevState;
		m_state = from.m_state;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalHistory::SignalHistory(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SignalHistory::~SignalHistory()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistory::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();

	emit signalCountChanged();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistory::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistory::append(const SignalForLog& signalLog)
{
	PS::Signal* pSignal = signalLog.signalPtr();
	if (pSignal == nullptr)
	{
		assert(false);
		return -1;
	}

	if (pSignal->hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		m_signalList.append(signalLog);
		index = m_signalList.count() - 1;

	m_signalMutex.unlock();

	emit signalCountChanged();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog* SignalHistory::signalPtr(int index) const
{
	SignalForLog* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = (SignalForLog*) &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog SignalHistory::signal(int index) const
{
	SignalForLog signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalHistory& SignalHistory::operator=(const SignalHistory& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalHistoryTable::SignalHistoryTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalHistoryTable::~SignalHistoryTable()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistoryTable::columnCount(const QModelIndex&) const
{
	return SIGNAL_HISTORY_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistoryTable::rowCount(const QModelIndex&) const
{
	return signalCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalHistoryTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SIGNAL_HISTORY_LIST_COLUMN_COUNT)
		{
			result = SignalHistoryListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SignalHistoryTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > SIGNAL_HISTORY_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	SignalForLog* pSignalLog = signalPtr(row);
	if (pSignalLog == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case SIGNAL_HISTORY_LIST_COLUMN_TIME:			result = Qt::AlignLeft;		break;
			case SIGNAL_HISTORY_LIST_COLUMN_CUSTOM_ID:	result = Qt::AlignLeft;		break;
			case SIGNAL_HISTORY_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case SIGNAL_HISTORY_LIST_COLUMN_APP_ID:		result = Qt::AlignLeft;		break;
			case SIGNAL_HISTORY_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case SIGNAL_HISTORY_LIST_COLUMN_PREV_STATE:	result = Qt::AlignCenter;	break;
			case SIGNAL_HISTORY_LIST_COLUMN_STATE:		result = Qt::AlignCenter;	break;
			case SIGNAL_HISTORY_LIST_COLUMN_EN_RANGE:		result = Qt::AlignCenter;	break;
			default:										assert(0);
		}

		return result;
	}

	if (role == Qt::TextColorRole)
	{
		if (column == SIGNAL_HISTORY_LIST_COLUMN_TIME || column == SIGNAL_HISTORY_LIST_COLUMN_PREV_STATE)
		{
			return QColor(Qt::gray);
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSignalLog);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalHistoryTable::text(int row, int column, SignalForLog* pSignalLog) const
{
	if (row < 0 || row >= signalCount())
	{
		return QString();
	}

	if (column < 0 || column > SIGNAL_HISTORY_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSignalLog == nullptr)
	{
		return QString();
	}

	PS::Signal* pSignal = pSignalLog->signalPtr();
	if (pSignal == nullptr || pSignal->hash() == UNDEFINED_HASH)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case SIGNAL_HISTORY_LIST_COLUMN_TIME:			result = pSignalLog->time();							break;
		case SIGNAL_HISTORY_LIST_COLUMN_CUSTOM_ID:	result = pSignal->customAppSignalID();					break;
		case SIGNAL_HISTORY_LIST_COLUMN_EQUIPMENT_ID:	result = pSignal->equipmentID();						break;
		case SIGNAL_HISTORY_LIST_COLUMN_APP_ID:		result = pSignal->appSignalID();						break;
		case SIGNAL_HISTORY_LIST_COLUMN_CAPTION:		result = pSignal->caption();							break;
		case SIGNAL_HISTORY_LIST_COLUMN_PREV_STATE:	result = pSignalLog->stateStr(pSignalLog->prevState());	break;
		case SIGNAL_HISTORY_LIST_COLUMN_STATE:		result = pSignalLog->stateStr(pSignalLog->state());		break;
		case SIGNAL_HISTORY_LIST_COLUMN_EN_RANGE:		result = pSignal->engineeringRangeStr();				break;
		default:										assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryTable::updateColumn(int column)
{
	if (column < 0 || column >= SIGNAL_HISTORY_LIST_COLUMN_COUNT)
	{
		return;
	}

	int count = rowCount();

	for (int row = 0; row < count; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// -------------------------------------------------------------------------------------------------------------------

int SignalHistoryTable::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog* SignalHistoryTable::signalPtr(int index) const
{
	SignalForLog* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			 pSignal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryTable::set(const QVector<SignalForLog*> list_add)
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

void SignalHistoryTable::clear()
{
	int count = m_signalList.count();
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

// -------------------------------------------------------------------------------------------------------------------

SignalHistoryDialog::SignalHistoryDialog(SignalHistory* pLog, QWidget *parent) :
	QDialog(parent),
	m_pLog(pLog)
{
	if (m_pLog != nullptr)
	{
		connect(m_pLog, &SignalHistory::signalCountChanged, this, &SignalHistoryDialog::updateList, Qt::QueuedConnection);
	}

	createInterface();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalHistoryDialog::~SignalHistoryDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/History.png"));
	setWindowTitle(tr("History"));
	resize(QApplication::desktop()->availableGeometry().width() - 900, 500);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());


	m_pMenuBar = new QMenuBar(this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pEditMenu->addSeparator();

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pCopyAction, &QAction::triggered, this, &SignalHistoryDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &SignalHistoryDialog::selectAll);

	m_pView = new QTableView(this);
	m_pView->setModel(&m_signalTable);
	m_pView->verticalHeader()->setDefaultSectionSize(22);

	for(int column = 0; column < SIGNAL_HISTORY_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, SignalHistoryListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);

	setLayout(mainLayout);

	createHeaderContexMenu();
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::createHeaderContexMenu()
{
	// init header context menu
	//
	m_pView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &SignalHistoryDialog::onHeaderContextMenu);

	m_headerContextMenu = new QMenu(m_pView);

	for(int column = 0; column < SIGNAL_HISTORY_LIST_COLUMN_COUNT; column++)
	{
		m_pColumnAction[column] = m_headerContextMenu->addAction(SignalHistoryListColumn[column]);
		if (m_pColumnAction[column] != nullptr)
		{
			m_pColumnAction[column]->setCheckable(true);
			m_pColumnAction[column]->setChecked(true);

			connect(m_headerContextMenu, static_cast<void (QMenu::*)(QAction*)>(&QMenu::triggered), this, &SignalHistoryDialog::onColumnAction);
		}
	}

	hideColumn(SIGNAL_HISTORY_LIST_COLUMN_CAPTION, true);
	hideColumn(SIGNAL_HISTORY_LIST_COLUMN_EN_RANGE, true);

}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCopyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableView::customContextMenuRequested, this, &SignalHistoryDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::updateList()
{
	if (m_pLog == nullptr)
	{
		return;
	}

	m_signalTable.clear();

	QVector<SignalForLog*> signalList;

	int count = m_pLog->count();
	for(int i = 0; i < count; i++)
	{
		SignalForLog* pSignalLog = m_pLog->signalPtr(i);
		if (pSignalLog == nullptr)
		{
			continue;
		}

		if (pSignalLog->signalPtr() == nullptr || pSignalLog->signalPtr()->hash() == UNDEFINED_HASH)
		{
			continue;
		}

		signalList.append(pSignalLog);
	}

	m_signalTable.set(signalList);

	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->setCurrentIndex(m_signalTable.index(m_signalTable.signalCount() - 1, SIGNAL_HISTORY_LIST_COLUMN_TIME));
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::hideColumn(int column, bool hide)
{
	if (column < 0 || column >= SIGNAL_HISTORY_LIST_COLUMN_COUNT)
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

void SignalHistoryDialog::copy()
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

void SignalHistoryDialog::onContextMenu(QPoint)
{
	if (m_pContextMenu == nullptr)
	{
		return;
	}

	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::onHeaderContextMenu(QPoint)
{
	if (m_headerContextMenu == nullptr)
	{
		return;
	}

	m_headerContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalHistoryDialog::onColumnAction(QAction* action)
{
	if (action == nullptr)
	{
		return;
	}

	for(int column = 0; column < SIGNAL_HISTORY_LIST_COLUMN_COUNT; column++)
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
