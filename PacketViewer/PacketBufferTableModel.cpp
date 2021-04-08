#include "PacketBufferTableModel.h"
#include "../OnlineLib/SocketIO.h"
#include <algorithm>
#include <QSettings>
#include "../OnlineLib/DataProtocols.h"

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
	m_frameDataSize(Rup::FRAME_DATA_SIZE),
	m_lastHeader(lastHeader),
	needToSwapBytes(true)
{
	QSettings settings;
	needToSwapBytes = settings.value("PacketBufferTableModel/needToSwapBytes", needToSwapBytes).toBool();
}

int PacketBufferTableModel::rowCount(const QModelIndex&) const
{
	return m_frameDataSize * m_lastHeader.partCount / sizeof(m_buffer[0]);
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

void PacketBufferTableModel::updateFrame(int frameNo, int version)
{
	int newFrameDataSize = 0;
	switch (version)
	{
		case 3:
			newFrameDataSize = RP_PACKET_DATA_SIZE;
			break;
		case 4:
		case 5:
			newFrameDataSize = Rup::FRAME_DATA_SIZE;
			break;
		default:
			assert(false);
	}

	if (m_frameDataSize != newFrameDataSize)
	{
		beginResetModel();
		m_frameDataSize = newFrameDataSize;
		endResetModel();
	}
	else
	{
		emit dataChanged(index(frameNo * m_frameDataSize / sizeof(m_buffer[0]), C_DECIMAL),
				index((frameNo + 1) * m_frameDataSize / sizeof(m_buffer[0]) - 1, C_BINARY),
				QVector<int>() << Qt::DisplayRole);
	}
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
							m_frameDataSize * m_lastHeader.partCount / sizeof(m_buffer[0]),
							m_frameDataSize * newPartCount / sizeof(m_buffer[0]) - 1);
	}
	if (newPartCount < m_lastHeader.partCount)
	{
		emit beginInsertRows(QModelIndex(),
							m_frameDataSize * newPartCount / sizeof(m_buffer[0]),
							m_frameDataSize * m_lastHeader.partCount / sizeof(m_buffer[0]) - 1);
	}
}

void PacketBufferTableModel::setNeedToSwapBytes(bool value)
{
	beginResetModel();
	needToSwapBytes = value;
	endResetModel();
	QSettings settings;
	settings.setValue("PacketBufferTableModel/needToSwapBytes", needToSwapBytes);
}

void PacketBufferTableModel::updateData()
{
	beginResetModel();
	endResetModel();
}

