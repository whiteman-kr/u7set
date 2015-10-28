#ifndef PACKETBUFFERTABLEMODEL_H
#define PACKETBUFFERTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class RpPacketHeader;

class PacketBufferTableModel : public QAbstractTableModel
{
public:
	PacketBufferTableModel(quint8* buffer, RpPacketHeader& lastHeader, QObject* parent);

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateFrame(int frameNo);
	void checkPartCount(int newPartCount);

signals:

public slots:

private:
	quint16* m_buffer;
	RpPacketHeader& m_lastHeader;
};

#endif // PACKETBUFFERTABLEMODEL_H
