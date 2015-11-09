#ifndef PACKETBUFFERTABLEMODEL_H
#define PACKETBUFFERTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>

template <typename TYPE>
void swapBytes(TYPE& value, int startPosition = -1, int count = -1)
{
	quint8* memory = reinterpret_cast<quint8*>(&value);
	if (startPosition == -1)
	{
		std::reverse(memory, memory + sizeof(TYPE));
	}
	else
	{
		if (static_cast<size_t>(startPosition) > sizeof(TYPE) - 1)
		{
			startPosition = sizeof(TYPE) - 1;
		}
		if (count == -1)
		{
			std::reverse(memory + startPosition, memory + sizeof(TYPE));
		}
		else
		{
			if (static_cast<size_t>(startPosition + count) > sizeof(TYPE))
			{
				count = sizeof(TYPE) - startPosition;
			}
			std::reverse(memory + startPosition, memory + startPosition + count);
		}
	}
}

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
	void setNeedToSwapBytes(bool value);

private:
	quint16* m_buffer;
	RpPacketHeader& m_lastHeader;
	bool needToSwapBytes;
};

#endif // PACKETBUFFERTABLEMODEL_H
