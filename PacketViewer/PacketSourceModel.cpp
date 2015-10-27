#include "PacketSourceModel.h"
#include <cassert>
#include <QWidget>
#include <QUdpSocket>
#include "../include/SocketIO.h"
#include "PacketBufferTableModel.h"
#include <QHBoxLayout>
#include <QTableView>

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

	return createIndex(row, column, const_cast<Source*>(&parent->source(row)));
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
	for (size_t i = 0; i < m_listeners.size(); i++)
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

int PacketSourceModel::columnCount(const QModelIndex&) const
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
		case 0: return statistic->fullAddress();
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

bool PacketSourceModel::canFetchMore(const QModelIndex&) const
{
	return false;
}

void PacketSourceModel::fetchMore(const QModelIndex&)
{
}

void PacketSourceModel::sort(int, Qt::SortOrder)
{

}

void PacketSourceModel::addListener(QString ip, int port)
{
	std::shared_ptr<Listener> listener(new Listener(this, ip, port));
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
	connect(listener.get(), &Listener::beginAddSource, this, &PacketSourceModel::beginInsertSource, Qt::DirectConnection);
	connect(listener.get(), &Listener::endAddSource, this, &PacketSourceModel::endInsertSource, Qt::DirectConnection);
	emit contentChanged(0);
}

int PacketSourceModel::index(Listener* listener)
{
	for (size_t i = 0; i < m_listeners.size(); i++)
	{
		if (m_listeners[i].get() == listener)
		{
			return i;
		}
	}
	return -1;
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

	Source* source = dynamic_cast<Source*>(statistic);
	source->openStatusWidget();
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

void PacketSourceModel::beginInsertSource(int row)
{
	Listener* listener = dynamic_cast<Listener*>(sender());
	if (listener == nullptr)
	{
		return;
	}
	beginInsertRows(index(index(listener), 0, QModelIndex()), row, row);
}

void PacketSourceModel::endInsertSource()
{
	endInsertRows();
}

void PacketSourceModel::updateSourceStatistic()
{
	Source* source = dynamic_cast<Source*>(sender());
	Listener* listener = dynamic_cast<Listener*>(source->parent());
	int listenerRow = index(listener);
	QModelIndex listenerIndex;
	emit dataChanged(listenerIndex = index(listenerRow, 0, QModelIndex()), index(listenerRow, columnCount() - 1, QModelIndex()));
	int sourceRow = listener->index(source);
	emit dataChanged(index(sourceRow, 0, listenerIndex), index(sourceRow, columnCount() - 1, listenerIndex));
}


Statistic::Statistic(Statistic* parent) :
	m_parent(parent)
{
}

Statistic::Statistic(QString address, int port, Statistic* parent) :
	m_fullAddress(address + ":" + QString::number(port)),
	m_ipAddress(QHostAddress(address).toIPv4Address()),
	m_port(port),
	m_parent(parent)
{
}


Listener::Listener() :
	Statistic(nullptr),
	m_model(nullptr)
{
}

Listener::Listener(PacketSourceModel* model, QString address, int port) :
	Statistic(address, port, nullptr),
	m_socket(new QUdpSocket(this)),
	m_model(model)
{
	assert(!address.isEmpty());
	connect(m_socket.get(), &QUdpSocket::readyRead, this, &Listener::readPendingDatagrams);
	m_socket->bind(QHostAddress(address), port);
}

int Listener::index(Source* source) const
{
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		if (m_sources[i].get() == source)
		{
			return i;
		}
	}
	return -1;
}

int Listener::getSourceIndex(quint32 ip, quint16 port)
{
	int sourceIndex = -1;
	if (m_sourcesMap.contains(ip))
	{
		sourceIndex = m_sourcesMap[ip];
		Source& source = *m_sources[sourceIndex];
		if (!source.isSameAddress(ip, port))
		{
			sourceIndex = -1;
			for (size_t i = 0; i < m_sources.size(); i++)
			{
				if (m_sources[i]->isSameAddress(ip, port))
				{
					return i;
				}
			}
		}
		else
		{
			return sourceIndex;
		}
	}
	return addNewSource(ip, port);
}

int Listener::addNewSource(quint32 ip, quint16 port)
{
	std::shared_ptr<Source> newSource(new Source(QHostAddress(ip).toString(), port, this));
	connect(newSource.get(), &Source::fieldsChanged, m_model, &PacketSourceModel::updateSourceStatistic);
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		if (m_sources[i]->ip() > ip || (m_sources[i]->ip() == ip && m_sources[i]->port() >= port))
		{
			emit beginAddSource(i);
			m_sources.insert(m_sources.begin() + i, newSource);
			emit endAddSource();
			updateSourceMap();
			return i;
		}
	}
	emit beginAddSource(m_sources.size());
	m_sources.push_back(newSource);
	emit endAddSource();
	updateSourceMap();
	return m_sources.size() - 1;
}

void Listener::updateSourceMap()
{
	m_sourcesMap.clear();
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		m_sourcesMap.insert(m_sources[i]->ip(), i);
	}
}

void Listener::readPendingDatagrams()
{
	QHostAddress senderAddress;
	quint16 senderPort;
	char* buffer = new char[ENTIRE_UDP_SIZE];
	quint64 readBytes = m_socket->readDatagram(buffer, ENTIRE_UDP_SIZE, &senderAddress, &senderPort);
	quint32 senderIp4 = senderAddress.toIPv4Address();

	int sourceIndex = getSourceIndex(senderIp4, senderPort);
	m_sources[sourceIndex]->parseReceivedBuffer(buffer, readBytes);	// source owns buffer from this moment
}


Source::Source(QString address, int port, Statistic* parent) :
	Statistic(address, port, parent),
	m_packetBufferModel(new PacketBufferTableModel(m_buffer, m_lastHeader, this))
{
	m_lastHeader.packetNo = 0;
	memset(m_buffer, 0, RP_MAX_FRAME_COUNT * RP_PACKET_DATA_SIZE);
}

Source::~Source()
{
	for (size_t i = 0; i < dependentWidgets.size(); i++)
	{
		dependentWidgets[i]->deleteLater();
	}
}

void Source::parseReceivedBuffer(char* buffer, quint64 readBytes)
{
	RpPacket& packet = *reinterpret_cast<RpPacket*>(buffer);
	RpPacketHeader& header = packet.Header;
	incrementPacketReceivedCount();
	if (readBytes != header.packetSize)
	{
		incrementPartialFrameCount();
	}
	if (header.partCount > RP_MAX_FRAME_COUNT || header.partNo >= header.partCount || header.packetSize > ENTIRE_UDP_SIZE)
	{
		incrementFormatErrorCount();
		delete [] buffer;
		return;
	}
	if (header.packetNo - m_lastHeader.packetNo > 1 && m_lastHeader.packetNo != 0)
	{
		incrementPacketLostCount(header.packetNo - m_lastHeader.packetNo);
	}
	// Check correct packet part sequence
	if (!((header.packetNo == m_lastHeader.packetNo && header.partNo == m_lastHeader.partNo + 1) ||
		(header.packetNo == m_lastHeader.packetNo + 1 && header.partNo == 0 && m_lastHeader.partNo == m_lastHeader.partCount - 1)))
	{
		incrementPartialPacketCount();
	}
	int currentDataSize = header.packetSize - sizeof(RpPacketHeader) - sizeof(packet.CRC64);
	memcpy(m_buffer + header.partNo * currentDataSize, packet.Data, currentDataSize);
	m_packetBufferModel->updateFrame(header.partNo);
	memcpy(&m_lastHeader, &header, sizeof(RpPacketHeader));
	emit fieldsChanged();
	delete [] buffer;
}

void Source::openStatusWidget()
{
	QWidget* widget = new QWidget();
	dependentWidgets.push_back(widget);
	widget->setWindowTitle(fullAddress());
	connect(widget, &QWidget::destroyed, this, &Source::removeDependentWidget);

	QTableView* table = new QTableView(widget);
	table->setModel(m_packetBufferModel);
	table->resizeColumnsToContents();

	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(table);
	widget->setLayout(layout);
	widget->resize(640, 480);
	widget->show();
	widget->setAttribute(Qt::WA_DeleteOnClose, true);
}

void Source::removeDependentWidget(QObject* object)
{
	QWidget* widget = dynamic_cast<QWidget*>(object);
	for (size_t i = 0; i < dependentWidgets.size(); i++)
	{
		if (dependentWidgets[i] == widget)
		{
			dependentWidgets.erase(dependentWidgets.begin() + i);
		}
	}
}
