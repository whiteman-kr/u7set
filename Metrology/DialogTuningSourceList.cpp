#include "DialogTuningSourceList.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant TuningSourceTable::data(const QModelIndex &index, int role) const
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

	TuningSource src = at(row);

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
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogTuningSourceList::DialogTuningSourceList(QWidget* parent) :
	DialogList(0.7, 0.4, false, parent)
{
	createInterface();
	DialogTuningSourceList::updateList();

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
	m_sourceTable.setColumnCaption(DialogTuningSourceList::metaObject()->className(), TUN_SOURCE_LIST_COLUMN_COUNT, TuningSourceColumn);
	setModel(&m_sourceTable);

	//
	//
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

	std::vector<TuningSource> sourceList;

	int souceCount = theSignalBase.tuning().sourceBase().count();
	for(int i = 0; i < souceCount; i++)
	{
		const TuningSource& src = theSignalBase.tuning().sourceBase().source(i);
		if (src.sourceID() == 0)
		{
			continue;
		}

		sourceList.push_back(src);
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

