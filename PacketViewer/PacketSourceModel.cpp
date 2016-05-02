#include "PacketSourceModel.h"
#include <cassert>
#include <QWidget>
#include <QUdpSocket>
#include "../include/SocketIO.h"
#include "PacketBufferTableModel.h"
#include <QHBoxLayout>
#include <QTableView>
#include "SignalTableModel.h"
#include <QSettings>
#include "SourceStatusWidget.h"
#include <QDirIterator>
#include <QMessageBox>
#include "../include/DataProtocols.h"
#include <QTimer>

PacketSourceModel::PacketSourceModel(QObject* parent) :
	QAbstractItemModel(parent)
{
	Hardware::Init();
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &PacketSourceModel::updateSourceStatisticByTimer);
	timer->start(100);
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
	for (int i = 0; i < static_cast<int>(m_listeners.size()); i++)
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
		return static_cast<int>(m_listeners.size());
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

void PacketSourceModel::addListener(QString ip, int port, bool saveList)
{
	std::shared_ptr<Listener> listener(new Listener(this, ip, port));
	for (size_t i = 0; i < m_listeners.size(); i++)
	{
		if (*m_listeners[i] == *listener)
		{
			return;
		}
	}
	beginInsertRows(QModelIndex(), static_cast<int>(m_listeners.size()), static_cast<int>(m_listeners.size()));
	m_listeners.push_back(listener);
	endInsertRows();
	connect(listener.get(), &Listener::beginAddSource, this, &PacketSourceModel::beginInsertSource, Qt::DirectConnection);
	connect(listener.get(), &Listener::endAddSource, this, &PacketSourceModel::endInsertSource, Qt::DirectConnection);
	if (saveList)
	{
		saveListenerList();
	}
	emit contentChanged(0);
}

int PacketSourceModel::index(Listener* listener)
{
	for (int i = 0; i < static_cast<int>(m_listeners.size()); i++)
	{
		if (m_listeners[i].get() == listener)
		{
			return i;
		}
	}
	return -1;
}

void PacketSourceModel::saveListenerList()
{
	QSettings settings;
	settings.beginWriteArray("PacketSourceModel/listenAddresses", static_cast<int>(m_listeners.size()));
	for (int i = 0; i < static_cast<int>(m_listeners.size()); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("ip", m_listeners[i]->ip());
		settings.setValue("port", m_listeners[i]->port());
	}
	settings.endArray();
}

std::shared_ptr<QUdpSocket> PacketSourceModel::getSocket(const QString& address, int port)
{
	for (auto listener : m_listeners)
	{
		if (listener->isListening(address, port))
		{
			return listener->getSocket();
		}
	}
	return std::shared_ptr<QUdpSocket>();
}

void PacketSourceModel::loadProject(const QString& projectPath)
{
	QDirIterator signalsIt(projectPath, QStringList() << "*appSignals.xml", QDir::Files, QDirIterator::Subdirectories);
	if (signalsIt.hasNext())
	{
		m_unitInfo.clear();
		m_signalSet.clear();
		SerializeSignalsFromXml(signalsIt.next(), m_unitInfo, m_signalSet);
	}
	else
	{
		QMessageBox::critical(nullptr, "Error", "Could not find appSignals.xml");
		return;
	}
	QDirIterator equipmentIt(projectPath, QStringList() << "*equipment.xml", QDir::Files, QDirIterator::Subdirectories);
	if (equipmentIt.hasNext())
	{
		m_deviceRoot.reset();
		SerializeEquipmentFromXml(equipmentIt.next(), m_deviceRoot);
	}
	else
	{
		QMessageBox::critical(nullptr, "Error", "Could not find equipment.xml");
		return;
	}
	m_dataSources.clear();

	InitDataSources(m_dataSources, m_deviceRoot.get(), m_signalSet);

	for (auto listener : m_listeners)
	{
		listener->reloadProject();
	}
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

void PacketSourceModel::removeListener(int row)
{
	if (static_cast<int>(m_listeners.size()) > row)
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

void PacketSourceModel::updateSourceStatisticByTimer()
{
	QModelIndex listenerIndex;
	for (int i = 0; i < m_listeners.size(); i++)
	{
		emit dataChanged(listenerIndex = index(i, 0, QModelIndex()), index(i, columnCount() - 1, QModelIndex()));
		for (int j = 0; j < m_listeners[i]->childCount(); j++)
		{
			emit dataChanged(index(j, 0, listenerIndex), index(j, columnCount() - 1, listenerIndex));
		}
	}
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
	for (int i = 0; i < static_cast<int>(m_sources.size()); i++)
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
			for (int i = 0; i < static_cast<int>(m_sources.size()); i++)
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
	std::shared_ptr<Source> newSource(new Source(QHostAddress(ip).toString(), port, m_model->signalSet(), m_model->dataSources(), this));

	//connect(newSource.get(), &Source::fieldsChanged, m_model, &PacketSourceModel::updateSourceStatistic);
	for (int i = 0; i < static_cast<int>(m_sources.size()); i++)
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
	emit beginAddSource(static_cast<int>(m_sources.size()));
	m_sources.push_back(newSource);
	emit endAddSource();
	updateSourceMap();
	return static_cast<int>(m_sources.size()) - 1;
}

void Listener::updateSourceMap()
{
	m_sourcesMap.clear();
	for (int i = 0; i < static_cast<int>(m_sources.size()); i++)
	{
		m_sourcesMap.insert(m_sources[i]->ip(), i);
	}
}

bool Listener::isListening(const QString& address, int port)
{
	return m_socket->localAddress().toString() == address && m_socket->localPort() == port;
}

void Listener::readPendingDatagrams()
{
	while (m_socket->hasPendingDatagrams())
	{
		QHostAddress senderAddress;
		quint16 senderPort;
		char* buffer = new char[ENTIRE_UDP_SIZE];
		quint64 readBytes = m_socket->readDatagram(buffer, ENTIRE_UDP_SIZE, &senderAddress, &senderPort);
		quint32 senderIp4 = senderAddress.toIPv4Address();

		int sourceIndex = getSourceIndex(senderIp4, senderPort);
		m_sources[sourceIndex]->parseReceivedBuffer(buffer, readBytes);	// source owns buffer from this moment
	}
}

void Listener::reloadProject()
{
	for (auto source : m_sources)
	{
		source->reloadProject();
	}
}


Source::Source(QString address, int port, const SignalSet& signalSet, const QHash<quint32, std::shared_ptr<AppDataSource> > &dataSources, Statistic* parent) :
	Statistic(address, port, parent),
	m_packetBufferModel(new PacketBufferTableModel(m_buffer, m_lastHeader, this)),
	m_signalTableModel(new SignalTableModel(m_buffer, signalSet, this)),
	m_dataSources(&dataSources)
{
	m_lastHeader.packetNo = -1;
	memset(m_buffer, 0, RUP_MAX_FRAME_COUNT * RUP_FRAME_DATA_SIZE);
	memset(&m_lastHeader, 0, sizeof(m_lastHeader));
}

Source::~Source()
{
	for (size_t i = 0; i < dependentWidgets.size(); i++)
	{
		dependentWidgets[i]->deleteLater();
	}
}

void V4toV3Header(RpPacketHeader& v3header, const RupFrameHeader& v4header) // copy assignment
{
	v3header.packetSize = v4header.frameSize;
	v3header.protocolVersion = v4header.protocolVersion;
	memcpy(&v3header.flags, &v4header.flags, sizeof(RpPacketFlags));
	v3header.moduleFactoryNo = v4header.dataId;
	v3header.moduleType = v4header.moduleType;
	v3header.subblockID = 0;
	v3header.packetNo = v4header.numerator;
	v3header.partCount = v4header.framesQuantity;
	v3header.partNo = v4header.frameNumber;
	memcpy(&v3header.TimeStamp, &v4header.TimeStamp, sizeof(RpTimeStamp));
}

void Source::parseReceivedBuffer(char* buffer, quint64 readBytes)
{
	RpPacket& packet = *reinterpret_cast<RpPacket*>(buffer);
	quint16 version = packet.Header.protocolVersion;
	bool needSwap = false;
	if (packet.Header.packetSize > ENTIRE_UDP_SIZE)
	{
		quint16 swapedPacketSize = packet.Header.packetSize;
		swapBytes(swapedPacketSize);
		if (swapedPacketSize == ENTIRE_UDP_SIZE)
		{
			needSwap = true;
			swapBytes(version);
		}
	}
	RpPacketHeader header;
	switch (version)
	{
		case 3:
			if (needSwap)
			{
				swapHeader(packet.Header);
			}
			header = packet.Header;
			break;
		case 4:
		{
			RupFrameHeader& v4Header = *reinterpret_cast<RupFrameHeader*>(buffer);
			if (needSwap)
			{
				swapHeader(v4Header);
			}
			V4toV3Header(header, v4Header);
			break;
		}
		default:
			assert(false);
	}

	//swapHeader(header);
	incrementPacketReceivedCount();
	if (readBytes != header.packetSize)
	{
		incrementPartialFrameCount();
	}

	if (header.partCount > RUP_MAX_FRAME_COUNT || header.partNo >= header.partCount || header.packetSize > ENTIRE_UDP_SIZE)
	{
		incrementFormatErrorCount();
		delete [] buffer;
		return;
	}

	int currentDataSize = 0;
	switch (version)
	{
		case 3:
			if (m_lastHeader.packetSize != 0 && header.packetNo > m_lastHeader.packetNo && header.packetNo - m_lastHeader.packetNo > header.partCount)
			{
				incrementPacketLostCount((header.packetNo - m_lastHeader.packetNo) / header.partCount);
			}
			// Check correct packet part sequence
			if (!((m_lastHeader.packetSize == 0) ||
				  (header.packetNo == m_lastHeader.packetNo + 1 && header.partNo == m_lastHeader.partNo + 1) ||
				  (header.packetNo == m_lastHeader.packetNo + 1 && header.partNo == 0 && m_lastHeader.partNo == m_lastHeader.partCount - 1)))
			{
				incrementPartialPacketCount();
			}
			currentDataSize = header.packetSize - sizeof(RpPacketHeader) - sizeof(packet.CRC64);
			memcpy(m_buffer + header.partNo * currentDataSize, packet.Data, currentDataSize);
			break;
		case 4:
			if (m_lastHeader.packetSize != 0 && header.packetNo > m_lastHeader.packetNo && header.packetNo - m_lastHeader.packetNo > 1)
			{
				incrementPacketLostCount(header.packetNo - m_lastHeader.packetNo - 1);
			}
			// Check correct frame sequence
			if (!((m_lastHeader.packetSize == 0) ||
				  (header.packetNo == m_lastHeader.packetNo && header.partNo == m_lastHeader.partNo + 1) ||
				  (header.packetNo == m_lastHeader.packetNo + 1 && header.partNo == 0 && m_lastHeader.partNo == m_lastHeader.partCount - 1)))
			{
				incrementPartialPacketCount();
			}

			currentDataSize = header.packetSize - sizeof(RupFrameHeader) - sizeof(packet.CRC64);
			memcpy(m_buffer + header.partNo * currentDataSize, buffer + sizeof(RupFrameHeader), currentDataSize);
			break;
		default:
			assert(false);
	}
	if (!dependentWidgets.empty())
	{
		m_signalTableModel->updateFrame(header.partNo);
		m_packetBufferModel->updateFrame(header.partNo, version);
		m_packetBufferModel->checkPartCount(header.partCount);
	}
	memcpy(&m_lastHeader, &header, sizeof(RpPacketHeader));
	emit fieldsChanged();
	delete [] buffer;
}

void Source::openStatusWidget()
{
	dependentWidgets.push_back(new SourceStatusWidget(*this, m_packetBufferModel, m_signalTableModel));
	if (dependentWidgets.size() == 1)
	{
		m_signalTableModel->updateData();
		m_packetBufferModel->updateData();
	}
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

void Source::reloadProject()
{
	m_signalTableModel->beginReloadProject();
	QHashIterator<quint32, std::shared_ptr<AppDataSource>> iterator(*m_dataSources);

	while (iterator.hasNext())
	{
		iterator.next();

		if (iterator.value()->lmAddress32() == ip())
		{
			m_signalTableModel->addDataSource(iterator.value().get());
		}
	}
	m_signalTableModel->endReloadProject();
}

void swapHeader(RpPacketHeader &header)
{
	swapBytes(header.packetSize);
	swapBytes(header.protocolVersion);
	swapBytes(header.flags);
	swapBytes(header.moduleFactoryNo);
	swapBytes(header.moduleType);
	swapBytes(header.subblockID);
	swapBytes(header.packetNo);
	swapBytes(header.partCount);
	swapBytes(header.partNo);
	swapBytes(header.TimeStamp.hour);
	swapBytes(header.TimeStamp.Minute);
	swapBytes(header.TimeStamp.Second);
	swapBytes(header.TimeStamp.Millisecond);
	swapBytes(header.TimeStamp.day);
	swapBytes(header.TimeStamp.month);
	swapBytes(header.TimeStamp.year);
}

void swapHeader(RupFrameHeader& header)
{
	swapBytes(header.frameSize);
	swapBytes(header.protocolVersion);
	swapBytes(header.flags);
	swapBytes(header.dataId);
	swapBytes(header.moduleType);
	swapBytes(header.numerator);
	swapBytes(header.framesQuantity);
	swapBytes(header.frameNumber);
	swapBytes(header.TimeStamp.hour);
	swapBytes(header.TimeStamp.minute);
	swapBytes(header.TimeStamp.second);
	swapBytes(header.TimeStamp.millisecond);
	swapBytes(header.TimeStamp.day);
	swapBytes(header.TimeStamp.month);
	swapBytes(header.TimeStamp.year);
}

void PacketSourceModel::InitDataSources(QHash<quint32, std::shared_ptr<AppDataSource> > &dataSources, Hardware::DeviceObject* deviceRoot, const SignalSet& signalSet)
{
	dataSources.clear();

	if (deviceRoot == nullptr)
	{
		return;
	}

	Hardware::equipmentWalker(deviceRoot, [&dataSources, &signalSet](Hardware::DeviceObject* currentDevice)
	{
		if (currentDevice == nullptr)
		{
			return;
		}
		if (typeid(*currentDevice) != typeid(Hardware::DeviceModule))
		{
			return;
		}
		Hardware::DeviceModule* currentModule = dynamic_cast<Hardware::DeviceModule*>(currentDevice);
		if (currentModule == nullptr)
		{
			return;
		}
		if (currentModule->moduleFamily() != Hardware::DeviceModule::LM)
		{
			return;
		}
		QStringList propertyList = QStringList() << "RegIP1" << "RegIP2";
		for (QString prop : propertyList)
		{
			if (currentModule->propertyValue(prop).isValid())
			{
				int key = dataSources.count() + 1;
				QString ipStr = currentModule->propertyValue(prop).toString();
				QHostAddress ha(ipStr);
				quint32 ip = ha.toIPv4Address();

				std::shared_ptr<AppDataSource> ds = std::make_shared<AppDataSource>();
				ds->setID(ip);
				ds->setLmCaption(QString("Data Source %1").arg(key));
				ds->setLmAddressStr(ha.toString());
				ds->partCount(1);

				QString signalPrefix = currentModule->parent()->equipmentId();
				int signalPrefixLength = signalPrefix.length();
				for (int i = 0; i < signalSet.count(); i++)
				{
					if (signalSet[i].equipmentID().left(signalPrefixLength) == signalPrefix)
					{
						ds->addSignalIndex(i);
					}
				}

				dataSources.insert(key, ds);
			}
		}
	});
}
