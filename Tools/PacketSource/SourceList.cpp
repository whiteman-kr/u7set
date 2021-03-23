#include "SourceList.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceTable::SourceTable(QObject *)
{
	startUpdateSourceListTimer();
}

// -------------------------------------------------------------------------------------------------------------------

SourceTable::~SourceTable()
{
	stopUpdateSourceListTimer();

	QMutexLocker l(&m_sourceMutex);

	m_sourceList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceTable::columnCount(const QModelIndex&) const
{
	return SOURCE_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int SourceTable::rowCount(const QModelIndex&) const
{
	return sourceCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SourceTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < SOURCE_LIST_COLUMN_COUNT)
		{
			result = SourceListColumn[section];
		}
	}

	if (orientation == Qt::Vertical)
	{
		//result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant SourceTable::data(const QModelIndex &index, int role) const
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
	if (column < 0 || column > SOURCE_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	PS::Source* pSource = sourceAt(row);
	if (pSource == nullptr)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return Qt::AlignCenter;
	}

	if (role == Qt::ForegroundRole)
	{
		if (column == SOURCE_LIST_COLUMN_STATE)
		{
			if (pSource->isRunning() == false)
			{
				return QColor(0xFF, 0x00, 0x00);
			}
		}

		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pSource);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString SourceTable::text(int row, int column, PS::Source* pSource) const
{
	if (row < 0 || row >= sourceCount())
	{
		return QString();
	}

	if (column < 0 || column > SOURCE_LIST_COLUMN_COUNT)
	{
		return QString();
	}

	if (pSource == nullptr)
	{
		return QString();
	}

	QString result;

	switch (column)
	{
		case SOURCE_LIST_COLUMN_LM_IP:			result = pSource->info().lmIP.addressStr() + " (" + QString::number(pSource->info().lmIP.port()) + ")";	break;
		case SOURCE_LIST_COLUMN_CAPTION:		result = pSource->info().caption;												break;
		case SOURCE_LIST_COLUMN_EQUIPMENT_ID:	result = pSource->info().equipmentID;											break;
		case SOURCE_LIST_COLUMN_MODULE_TYPE:	result = QString::number(pSource->info().moduleType);							break;
		case SOURCE_LIST_COLUMN_SUB_SYSTEM:		result = pSource->info().subSystem;												break;
		case SOURCE_LIST_COLUMN_FRAME_COUNT:	result = QString::number(pSource->info().frameCount);							break;
		case SOURCE_LIST_COLUMN_STATE:			result = pSource->isRunning() ? QString::number(pSource->sentFrames()) : tr("Stopped");								break;
		case SOURCE_LIST_COLUMN_SIGNAL_COUNT:	result = QString::number(pSource->info().signalCount);							break;
		default:								assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::updateColumn(int column)
{
	if (column < 0 || column >= SOURCE_LIST_COLUMN_COUNT)
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

int SourceTable::sourceCount() const
{
	QMutexLocker l(&m_sourceMutex);

	return TO_INT(m_sourceList.size());
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceTable::sourceAt(int index) const
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return nullptr;
	}

	return m_sourceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::set(const std::vector<PS::Source*> list_add)
{
	int count = TO_INT(list_add.size());
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceList = list_add;

		m_sourceMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::clear()
{
	int count = TO_INT(m_sourceList.size());
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_sourceMutex.lock();

			m_sourceList.clear();

		m_sourceMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::startUpdateSourceListTimer()
{
	if (m_updateSourceListTimer == nullptr)
	{
		m_updateSourceListTimer = new QTimer(this);
		connect(m_updateSourceListTimer, &QTimer::timeout, this, &SourceTable::updateSourceState);
	}

	m_updateSourceListTimer->start(UPDATE_SOURCE_STATE_TIMEOUT);
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::stopUpdateSourceListTimer()
{
	if (m_updateSourceListTimer == nullptr)
	{
		return;
	}

	m_updateSourceListTimer->stop();
	delete m_updateSourceListTimer;
	m_updateSourceListTimer = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceTable::updateSourceState()
{
	updateColumn(SOURCE_LIST_COLUMN_STATE);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
