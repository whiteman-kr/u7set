#include "DialogTuningSourceList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSourceTable::TuningSourceTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceTable::~TuningSourceTable()
{
	QMutexLocker l(&m_sourceMutex);

	m_sourceIdList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceTable::columnCount(const QModelIndex&) const
{
	return TUN_SOURCE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceTable::rowCount(const QModelIndex&) const
{
	return sourceCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < TUN_SOURCE_LIST_COLUMN_COUNT)
		{
			result = qApp->translate("DialogTuningSourceList", TuningSourceColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= sourceCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	TuningSource src = source(row);

	TuningSourceState sourceState = src.state();

	if (	column == TUN_SOURCE_LIST_COLUMN_IS_REPLY ||
			column == TUN_SOURCE_LIST_COLUMN_REQUESTS ||
			column == TUN_SOURCE_LIST_COLUMN_REPLIES ||
			column == TUN_SOURCE_LIST_COLUMN_COMMANDS)
	{
		// get fresh state from base
		//
		 sourceState = theSignalBase.tuning().sourceBase().state(src.sourceID());
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = Qt::AlignLeft;		break;
			case TUN_SOURCE_LIST_COLUMN_CAPTION:		result = Qt::AlignLeft;		break;
			case TUN_SOURCE_LIST_COLUMN_IP:				result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_CHANNEL:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_IS_REPLY:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_REQUESTS:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_REPLIES:		result = Qt::AlignCenter;	break;
			case TUN_SOURCE_LIST_COLUMN_COMMANDS:		result = Qt::AlignCenter;	break;
			default:									assert(0);
		}

		return result;
	}

	if (role == Qt::ForegroundRole)
	{
		if (	column == TUN_SOURCE_LIST_COLUMN_REQUESTS ||
				column == TUN_SOURCE_LIST_COLUMN_REPLIES ||
				column == TUN_SOURCE_LIST_COLUMN_COMMANDS)
		{
			if (sourceState.isReply() == false)
			{
				return QColor(Qt::darkGray);
			}
		}

		return QVariant();
	}


	if (role == Qt::BackgroundRole)
	{
		if (column == TUN_SOURCE_LIST_COLUMN_IS_REPLY)
		{
			if (sourceState.isReply() == false)
			{
				return QColor(0xFF, 0xA0, 0xA0) ;
			}
			else
			{
				return QColor(0xA0, 0xFF, 0xA0) ;
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, src, sourceState);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSourceTable::text(int row, int column, const TuningSource& source, const TuningSourceState& state) const
{
	if (row < 0 || row >= sourceCount())
	{
		return QString();
	}

	if (column < 0 || column > TUN_SOURCE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = source.equipmentID();							break;
		case TUN_SOURCE_LIST_COLUMN_CAPTION:		result = source.caption();								break;
		case TUN_SOURCE_LIST_COLUMN_IP:				result = source.serverIP() + " (" + QString::number(source.serverPort()) + ")";	break;
		case TUN_SOURCE_LIST_COLUMN_CHANNEL:		result = source.channel();								break;
		case TUN_SOURCE_LIST_COLUMN_SUBSYSTEM:		result = source.subSystem();							break;
		case TUN_SOURCE_LIST_COLUMN_LM_NUMBER:		result = QString::number(source.lmNumber());			break;
		case TUN_SOURCE_LIST_COLUMN_IS_REPLY:		result = state.isReply() == false ? "No" : "Yes";		break;
		case TUN_SOURCE_LIST_COLUMN_REQUESTS:		result = QString::number(state.requestCount());			break;
		case TUN_SOURCE_LIST_COLUMN_REPLIES:		result = QString::number(state.replyCount());			break;
		case TUN_SOURCE_LIST_COLUMN_COMMANDS:		result = QString::number(state.commandQueueSize());		break;
		default:									assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::updateColumn(int column)
{
	if (column < 0 || column >= TUN_SOURCE_LIST_COLUMN_COUNT)
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

int TuningSourceTable::sourceCount() const
{
	QMutexLocker l(&m_sourceMutex);

	return m_sourceIdList.count();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource TuningSourceTable::source(int index) const
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= m_sourceIdList.count())
	{
		return TuningSource();
	}

	return m_sourceIdList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::set(const QVector<TuningSource>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceIdList = list_add;

		m_sourceMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceTable::clear()
{
	int count = m_sourceIdList.count();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceIdList.clear();

		m_sourceMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogTuningSourceList::DialogTuningSourceList(QWidget* parent) :
	DialogList(0.7, 0.4, false, parent)
{
	createInterface();
	updateList();

	startSourceStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

DialogTuningSourceList::~DialogTuningSourceList()
{
	stopSourceStateTimer();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::createInterface()
{
	setWindowTitle(tr("Tuning sources"));

	// menu
	//
	m_pSourceMenu = new QMenu(tr("&Source"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	// action
	//
	m_pSourceMenu->addAction(m_pExportAction);

	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();

	//
	//
	addMenu(m_pSourceMenu);
	addMenu(m_pEditMenu);

	//
	//
	setModel(&m_sourceTable);
	DialogList::createHeaderContexMenu(TUN_SOURCE_LIST_COLUMN_COUNT, TuningSourceColumn, TuningSourceColumnWidth);

	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::createContextMenu()
{
	addContextAction(m_pCopyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::updateList()
{
	// update source list
	//
	m_sourceTable.clear();

	QVector<TuningSource> sourceList;

	int souceCount = theSignalBase.tuning().sourceBase().count();
	for(int i = 0; i < souceCount; i++)
	{
		const TuningSource& src = theSignalBase.tuning().sourceBase().source(i);
		if (src.sourceID() == 0)
		{
			continue;
		}

		sourceList.append(src);
	}

	m_sourceTable.set(sourceList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::updateState()
{
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_IS_REPLY);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REQUESTS);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_REPLIES);
	m_sourceTable.updateColumn(TUN_SOURCE_LIST_COLUMN_COMMANDS);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::startSourceStateTimer()
{
	if (m_updateSourceStateTimer == nullptr)
	{
		m_updateSourceStateTimer = new QTimer(this);
		connect(m_updateSourceStateTimer, &QTimer::timeout, this, &DialogTuningSourceList::updateState);
	}

	m_updateSourceStateTimer->start(250); // 250 ms
}

// -------------------------------------------------------------------------------------------------------------------

void DialogTuningSourceList::stopSourceStateTimer()
{
	if (m_updateSourceStateTimer != nullptr)
	{
		m_updateSourceStateTimer->stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

