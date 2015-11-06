#include "PacketBufferTableModel.h"
#include <../include/SocketIO.h>
#include <algorithm>
#include <QSettings>

const int C_DECIMAL = 0,
C_HEXADECIMAL = 1,
C_BINARY = 2,
C_COUNT = 3;

const char* const Columns[] =
{
	"Decimal",
	"Hexadecimal",
	"Binary",
};

PacketBufferTableModel::PacketBufferTableModel(quint8* buffer, RpPacketHeader& lastHeader, QObject* parent) :
	QAbstractTableModel(parent),
	m_buffer(reinterpret_cast<quint16*>(buffer)),
	m_lastHeader(lastHeader),
	needToSwapBytes(true)
{
	QSettings settings;
	needToSwapBytes = settings.value("needToSwapBytes", needToSwapBytes).toBool();
}

template <typename TYPE>
void swapBytes(TYPE& value)
{
	quint8* memory = reinterpret_cast<quint8*>(&value);
	std::reverse(memory, memory + sizeof(TYPE));
}

int PacketBufferTableModel::rowCount(const QModelIndex&) const
{
	return RP_PACKET_DATA_SIZE * m_lastHeader.partCount / sizeof(m_buffer[0]);
}

int PacketBufferTableModel::columnCount(const QModelIndex&) const
{
	return C_COUNT;
}

QVariant PacketBufferTableModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		quint16 value = m_buffer[index.row()];
		if (needToSwapBytes)
		{
			swapBytes(value);
		}
		switch (index.column())
		{
			case C_DECIMAL: return value;
			case C_HEXADECIMAL: return QString("%1").arg(value, 4, 16, QChar('0'));
			case C_BINARY:
			{
				QString result = QString("%1").arg(value, 16, 2, QChar('0'));
				for (int i = 3; i > 0; i--)
				{
					result.insert(i * 4, QChar(' '));
				}
				return result;
			}
		}
	}
	return QVariant();
}

QVariant PacketBufferTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return QString(Columns[section]);
		}
		if (orientation == Qt::Vertical)
		{
			return section;
		}
	}
	return QVariant();
}

void PacketBufferTableModel::updateFrame(int frameNo)
{
	emit dataChanged(index(frameNo * RP_PACKET_DATA_SIZE / sizeof(m_buffer[0]), C_DECIMAL),
			index((frameNo + 1) * RP_PACKET_DATA_SIZE / sizeof(m_buffer[0]) - 1, C_BINARY),
			QVector<int>() << Qt::DisplayRole);
}

void PacketBufferTableModel::checkPartCount(int newPartCount)
{
	if (newPartCount == m_lastHeader.partCount)
	{
		return;
	}
	if (newPartCount > m_lastHeader.partCount)
	{
		emit beginInsertRows(QModelIndex(),
							RP_PACKET_DATA_SIZE * m_lastHeader.partCount / sizeof(m_buffer[0]),
							RP_PACKET_DATA_SIZE * newPartCount / sizeof(m_buffer[0]) - 1);
	}
	if (newPartCount < m_lastHeader.partCount)
	{
		emit beginInsertRows(QModelIndex(),
							RP_PACKET_DATA_SIZE * newPartCount / sizeof(m_buffer[0]),
							RP_PACKET_DATA_SIZE * m_lastHeader.partCount / sizeof(m_buffer[0]) - 1);
	}
}

void PacketBufferTableModel::setNeedToSwapBytes(bool value)
{
	beginResetModel();
	needToSwapBytes = value;
	endResetModel();
	QSettings settings;
	settings.setValue("needToSwapBytes", needToSwapBytes);
}

