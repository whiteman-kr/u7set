#ifndef PACKETBUFFERTABLEMODEL_H
#define PACKETBUFFERTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

template <typename TYPE>
void swapBytes(TYPE& value, int count = -1)
{
	quint8* memory = reinterpret_cast<quint8*>(&value);
	if (static_cast<size_t>(count) > sizeof(TYPE) || count < 0)
	{
		count = sizeof(TYPE);
	}
	std::reverse(memory, memory + count);
}

struct RpPacketHeader;

class PacketBufferTableModel : public QAbstractTableModel
{
public:
	PacketBufferTableModel(quint8* buffer, RpPacketHeader& lastHeader, QObject* parent);

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void updateFrame(int frameNo, int version);
	void checkPartCount(int newPartCount);

signals:

public slots:
	void setNeedToSwapBytes(bool value);
	void updateData();

private:
	quint16* m_buffer;
	int m_frameDataSize;
	RpPacketHeader& m_lastHeader;
	bool needToSwapBytes;
};

#endif // PACKETBUFFERTABLEMODEL_H
