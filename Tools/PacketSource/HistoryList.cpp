#include "HistoryList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalHistoryTable::SignalHistoryTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalHistoryTable::~SignalHistoryTable()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
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

	if (role == Qt::ForegroundRole)
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
	QMutexLocker l(&m_signalMutex);

	return m_signalList.count();
}

// -------------------------------------------------------------------------------------------------------------------

SignalForLog* SignalHistoryTable::signalPtr(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= m_signalList.count())
	{
		return nullptr;
	}

	return m_signalList[index];
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

	QScreen* screenAt = QGuiApplication::primaryScreen();
	if (screenAt != nullptr)
	{
		resize(screenAt->availableGeometry().width() - 900, 500);
		move(screenAt->availableGeometry().center() - rect().center());
	}

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

	hideColumn(SIGNAL_HISTORY_LIST_COLUMN_CUSTOM_ID, true);
	hideColumn(SIGNAL_HISTORY_LIST_COLUMN_EQUIPMENT_ID, true);
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
