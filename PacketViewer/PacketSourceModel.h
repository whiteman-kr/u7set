#ifndef PACKETSOURCEMODEL_H
#define PACKETSOURCEMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include <unordered_map>
#include "../include/Signal.h"

class QUdpSocket;
class PacketSourceModel;
class PacketBufferTableModel;

#define DECLARE_INCREMENTER(function, member) void function() \
{ \
	if (parent() != nullptr) \
		parent()->function(); \
	member++; \
}

class Statistic : public QObject
{
	Q_OBJECT
public:
	Statistic(Statistic* parent);
	Statistic(QString address, int port, Statistic* parent);
	virtual ~Statistic() {}

	Statistic* parent() { return m_parent; }

	QString fullAddress() const { return m_fullAddress; }
	quint32 ip() const { return m_ipAddress; }
	quint16 port() const { return m_port; }
	int packetReceivedCount() const { return m_packetReceivedCount; }
	int packetLostCount() const { return m_packetLostCount; }
	int partialPacketCount() const { return m_partialPacketCount; }
	int partialFrameCount() const { return m_partialFrameCount; }
	int formatErrorCount() const { return m_formatErrorCount; }

	DECLARE_INCREMENTER(incrementPacketReceivedCount, m_packetReceivedCount)
	DECLARE_INCREMENTER(incrementPacketLostCount, m_packetLostCount)
	DECLARE_INCREMENTER(incrementPartialPacketCount, m_partialPacketCount)
	DECLARE_INCREMENTER(incrementPartialFrameCount, m_partialFrameCount)
	DECLARE_INCREMENTER(incrementFormatErrorCount, m_formatErrorCount)

	void incrementPacketLostCount(int difference)
	{
		if (parent() != nullptr)
		{
			parent()->incrementPacketLostCount(difference);
		}
		m_packetLostCount += difference;
	}

	virtual int childCount() = 0;

	virtual bool operator ==(const Statistic& s) const
	{
		return m_ipAddress == s.m_ipAddress && m_port == s.m_port;
	}

	bool isSameAddress(quint32 ip, quint16 port) const
	{
		return ip == m_ipAddress && port == m_port;
	}

private:
	QString m_fullAddress;
	quint32 m_ipAddress;
	quint16 m_port;
	int m_packetReceivedCount = 0;
	int m_partialFrameCount = 0;
	int m_packetLostCount = 0;
	int m_partialPacketCount = 0;
	int m_formatErrorCount = 0;
	Statistic* m_parent;
};


class Source : public Statistic
{
	Q_OBJECT
public:
	Source() : Statistic(nullptr) {}
	Source(QString address, int port, Statistic* parent);

	~Source();

	int childCount() { return 0; }

	void parseReceivedBuffer(char* buffer, quint64 readBytes);
	void openStatusWidget();
	void removeDependentWidget(QObject* object);

signals:
	void fieldsChanged();

private:
	quint8 m_buffer[RP_MAX_FRAME_COUNT * RP_PACKET_DATA_SIZE];
	RpPacketHeader m_lastHeader;
	std::vector<QWidget*> dependentWidgets;
	PacketBufferTableModel* m_packetBufferModel;
	//should be SignalTableModel;
};


class Listener : public Statistic
{
	Q_OBJECT
public:
	Listener();
	Listener(PacketSourceModel* model, QString address, int port);

	Source& source(int index) const { return *m_sources[index].get(); }
	int index(Source* source) const;

	int childCount() { return m_sources.size(); }

	int getSourceIndex(quint32 ip, quint16 port);
	int addNewSource(quint32 ip, quint16 port);	// returns index of new source
	void updateSourceMap();

signals:
	void beginAddSource(int row);
	void endAddSource();

public slots:
	void readPendingDatagrams();

private:
	std::vector<std::shared_ptr<Source>> m_sources;
	QHash<quint32, int> m_sourcesMap;
	std::shared_ptr<QUdpSocket> m_socket;
	PacketSourceModel* m_model;
};


class PacketSourceModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	PacketSourceModel(QObject* parent);
	virtual ~PacketSourceModel();

	QModelIndex index(int row, const QModelIndex& parentIndex) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual bool canFetchMore(const QModelIndex& parent) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	void addListener(QString ip, int port);
	int index(Listener* listener);

signals:
	void contentChanged(int column);

public slots:
	void openSourceStatusWidget(const QModelIndex &index);
	void removeListener(size_t row);

	void beginInsertSource(int row);
	void endInsertSource();

	void updateSourceStatistic();

private:
	std::vector<std::shared_ptr<Listener>> m_listeners;

	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	SignalSet m_signalSet;
	UnitList m_unitInfo;

	QHash<quint32, DataSource> m_dataSources;
};

#endif // PACKETSOURCEMODEL_H
