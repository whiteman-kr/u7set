#ifndef PACKETSOURCEMODEL_H
#define PACKETSOURCEMODEL_H

#include <QAbstractItemModel>
#include <memory>
#include "../include/Signal.h"


class Statistic
{
public:
	Statistic(Statistic* parent);
	Statistic(QString address, int port, Statistic* parent);
	virtual ~Statistic() {}

	Statistic* parent() { return m_parent; }

	QString address() const { return m_address; }
	int packetReceivedCount() const { return m_packetReceivedCount; }
	int packetLostCount() const { return m_packetLostCount; }
	int partialPacketCount() const { return m_partialPacketCount; }
	int partialFrameCount() const { return m_partialFrameCount; }
	int formatErrorCount() const { return m_formatErrorCount; }

	void incrementPacketReceivedCount() { m_packetReceivedCount++; }
	void incrementPacketLostCount() { m_packetLostCount++; }
	void incrementPartialPacketCount() { m_partialPacketCount++; }
	void incrementPartialFrameCount() { m_partialFrameCount++; }
	void incrementFormatErrorCount() { m_formatErrorCount++; }

	virtual int childCount() = 0;

	virtual bool operator ==(const Statistic& s) const
	{
		return m_address == s.address();
	}

private:
	QString m_address;
	int m_packetReceivedCount = 0;
	int m_partialFrameCount = 0;
	int m_packetLostCount = 0;
	int m_partialPacketCount = 0;
	int m_formatErrorCount = 0;
	Statistic* m_parent;
};


class Source : Statistic
{
public:
	Source() : Statistic(nullptr) {}
	Source(QString address, int port, Statistic* parent);

	int childCount() { return 0; }
};


class Listener : public Statistic
{
public:
	Listener();
	Listener(QString address, int port);

	const Source& teller(int index) const { return *m_tellers[index].get(); }

	int childCount() { return m_tellers.size(); }

private:
	std::vector<std::shared_ptr<Source>> m_tellers;
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

signals:
	void contentChanged(int column);

public slots:
	void openSourceStatusWidget(const QModelIndex &index);
	void removeListener(size_t row);

private:
	std::vector<std::shared_ptr<Listener>> m_listeners;

	std::shared_ptr<Hardware::DeviceRoot> m_deviceRoot;
	SignalSet m_signalSet;
	UnitList m_unitInfo;

	QHash<quint32, DataSource> m_dataSources;
};

#endif // PACKETSOURCEMODEL_H
