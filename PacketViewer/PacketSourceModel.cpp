#include "PacketSourceModel.h"
#include <cassert>
#include <QWidget>

PacketSourceModel::PacketSourceModel(QObject* parent) :
	QAbstractItemModel(parent)
{
	SerializeEquipmentFromXml(m_deviceRoot);
	SerializeSignalsFromXml(m_unitInfo, m_signalSet);
	InitDataSources(m_dataSources, m_deviceRoot.get(), m_signalSet);
}

PacketSourceModel::~PacketSourceModel()
{

}

QModelIndex PacketSourceModel::index(int row, const QModelIndex& parentIndex) const
{
	return index(row, 0, parentIndex);
}

QModelIndex PacketSourceModel::index(int row, int column, const QModelIndex& parentIndex) const
{
	if (hasIndex(row, column, parentIndex) == false)
	{
		return QModelIndex();
	}

	if (parentIndex.isValid() == false)
	{
		return createIndex(row, column, const_cast<Listener*>(m_listeners[row].get()));
	}

	Listener* parent = static_cast<Listener*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(false);
		return QModelIndex();
	}

	return createIndex(row, column, const_cast<Source*>(&parent->teller(row)));
}

QModelIndex PacketSourceModel::parent(const QModelIndex& childIndex) const
{
	if (childIndex.isValid() == false)
	{
		return QModelIndex();
	}

	Statistic* child = static_cast<Statistic*>(childIndex.internalPointer());

	if (child == nullptr)
	{
		assert(false);
		return QModelIndex();
	}

	if (child->parent() == nullptr)
	{
		return QModelIndex();
	}
	for (int i = 0; i < m_listeners.size(); i++)
	{
		if ((*child->parent()) == *m_listeners[i])
		{
			return createIndex(i, 0, child->parent());
		}
	}
	return QModelIndex();
}

int PacketSourceModel::rowCount(const QModelIndex& parentIndex) const
{
	if (!parentIndex.isValid())
	{
		return m_listeners.size();
	}
	else
	{
		Statistic* parent = static_cast<Statistic*>(parentIndex.internalPointer());

		if (parent == nullptr)
		{
			assert(false);
			return false;
		}

		return parent->childCount();
	}
}

int PacketSourceModel::columnCount(const QModelIndex& parent) const
{
	return 6;
}

QVariant PacketSourceModel::data(const QModelIndex& index, int role) const
{
	Statistic* statistic = static_cast<Statistic*>(index.internalPointer());

	if (statistic == nullptr)
	{
		assert(false);
		return QVariant();
	}
	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case 0: return statistic->address();
		case 1: return statistic->packetReceivedCount();
		case 2: return statistic->packetLostCount();
		case 3: return statistic->partialPacketCount();
		case 4: return statistic->partialFrameCount();
		case 5: return statistic->formatErrorCount();
		}
	}

	return QVariant();
}

QVariant PacketSourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case 0: return "IP";
			case 1: return "Received";
			case 2: return "Lost";
			case 3: return "Partial packets";
			case 4: return "Partial frames";
			case 5: return "Format errors";
			}
		}
	}
	return QVariant();
}

bool PacketSourceModel::hasChildren(const QModelIndex& parentIndex) const
{
	if (!parentIndex.isValid())
	{
		return !m_listeners.empty();
	}

	Statistic* parent = static_cast<Statistic*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(false);
		return false;
	}

	return parent->childCount() != 0;
}

bool PacketSourceModel::canFetchMore(const QModelIndex& parent) const
{
	return false;
}

void PacketSourceModel::fetchMore(const QModelIndex& parent)
{
}

void PacketSourceModel::sort(int column, Qt::SortOrder order)
{

}

void PacketSourceModel::addListener(QString ip, int port)
{
	std::shared_ptr<Listener> listener(new Listener(ip, port));
	for (size_t i = 0; i < m_listeners.size(); i++)
	{
		if (*m_listeners[i] == *listener)
		{
			return;
		}
	}
	beginInsertRows(QModelIndex(), m_listeners.size(), m_listeners.size());
	m_listeners.push_back(listener);
	endInsertRows();
	emit contentChanged(0);
}


void PacketSourceModel::openSourceStatusWidget(const QModelIndex& index)
{

	if (index.isValid() == false)
	{
		return;
	}

	Statistic* statistic = static_cast<Statistic*>(index.internalPointer());

	if (statistic == nullptr)
	{
		assert(false);
		return;
	}

	if (statistic->parent() == nullptr)
	{
		return;
	}

	QWidget* widget = new QWidget();
	widget->setWindowTitle(statistic->address());
	widget->show();
}

void PacketSourceModel::removeListener(size_t row)
{
	if (m_listeners.size() > row)
	{
		beginRemoveRows(QModelIndex(), row, row);
		m_listeners.erase(m_listeners.begin() + row);
		endRemoveRows();
	}
}


Statistic::Statistic(Statistic* parent) :
	m_address(),
	m_parent(parent)
{
}

Statistic::Statistic(QString address, int port, Statistic* parent) :
	m_address(address + ":" + QString::number(port)),
	m_parent(parent)
{
}


Listener::Listener() :
	Statistic(nullptr)
{
	m_tellers.push_back(std::shared_ptr<Source>(new Source(":)", 1, this)));
}

Listener::Listener(QString address, int port) :
	Statistic(address, port, nullptr)
{
	m_tellers.push_back(std::shared_ptr<Source>(new Source(":)", 1, this)));
}


Source::Source(QString address, int port, Statistic* parent) :
	Statistic(address, port, parent)
{

}
