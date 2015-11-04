#include "SignalTableModel.h"
#include "../include/Signal.h"

const int C_STR_ID = 0,
C_DESCRIPTION = 1,
C_ADC = 2,
C_VALUE = 3,
C_REG_ADDR = 4,
C_DATA_SIZE = 5,
C_COUNT = 6;

const char* const Columns[C_COUNT] =
{
	"ID",
	"Description",
	"ADC",
	"Value",
	"Registration address",
	"Data size"
};

SignalTableModel::SignalTableModel(quint8* buffer, const SignalSet& signalSet, QObject* parent) :
	QAbstractTableModel(parent),
	m_buffer(reinterpret_cast<quint16*>(buffer)),
	m_signalSet(signalSet)
{
}

SignalTableModel::~SignalTableModel()
{

}

int SignalTableModel::rowCount(const QModelIndex&) const
{
	return m_relatedSignalIndexes.size();
}

int SignalTableModel::columnCount(const QModelIndex&) const
{
	return C_COUNT;
}

QVariant SignalTableModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		const Signal& signal = m_signalSet[m_relatedSignalIndexes.at(index.row())];
		switch (index.column())
		{
			case C_STR_ID: return signal.strID();
			case C_DESCRIPTION: return signal.name();
			case C_ADC:
			{
				if (!signal.regAddr().isValid() || (signal.regAddr().offset() + (signal.regAddr().bit() + signal.dataSize()) / 8 > RP_BUFFER_SIZE))
				{
					return "???";
				}
				if (signal.isAnalog())
				{
					return QString("0x%1").arg(getAdc(signal), signal.dataSize() / 4, 16, QChar('0'));
				}
				else
				{
					return QString("B%1").arg(getAdc(signal), signal.dataSize(), 2, QChar('0'));
				}
			}
			case C_VALUE:
			{
				if (!signal.regAddr().isValid() || (signal.regAddr().offset() + (signal.regAddr().bit() + signal.dataSize()) / 8 > RP_BUFFER_SIZE))
				{
					return "???";
				}
				quint64 adc = getAdc(signal);
				if (signal.isAnalog())
				{
					return signal.lowLimit() + (adc - signal.lowADC()) * (signal.highLimit() - signal.lowLimit()) / (signal.highADC() - signal.lowADC());
				}
				else
				{
					return (adc == 0) ? 0 : 1;
				}
			}
			case C_REG_ADDR: return signal.regAddr().toString();
			case C_DATA_SIZE: return signal.dataSize();
			default: return QVariant();
		}
	}

	return QVariant();
}

QVariant SignalTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		if (orientation == Qt::Horizontal)
		{
			return QString(Columns[section]);
		}
		if (orientation == Qt::Vertical)
		{
			return m_signalSet[m_relatedSignalIndexes.at(section)].ID();
		}
	}
	return QVariant();
}

void SignalTableModel::updateFrame(int frameNo)
{
	if (m_signalSet.isEmpty() || m_frameSignalIndexLimits.empty())
	{
		return;
	}
	std::pair<int, int> limits = m_frameSignalIndexLimits[frameNo];
	if (limits.first != -1 && limits.second != -1)
	{
		emit dataChanged(index(limits.first, C_VALUE), index(limits.second, C_VALUE), QVector<int>() << Qt::DisplayRole);
	}
}

void SignalTableModel::addDataSource(const DataSource& dataSource)
{
	m_relatedSignalIndexes += dataSource.signalIndexes();
	std::sort(m_relatedSignalIndexes.begin(), m_relatedSignalIndexes.end(),
			  [this](int index1, int index2)
	{
		return m_signalSet[index1].regAddr().offset() < m_signalSet[index2].regAddr().offset() ||
				(m_signalSet[index1].regAddr().offset() == m_signalSet[index2].regAddr().offset() &&
				 m_signalSet[index1].regAddr().bit() < m_signalSet[index2].regAddr().bit());
	});
	for (int i = 0; i < m_relatedSignalIndexes.size() - 1; i++)
	{
		if (m_relatedSignalIndexes[i] == m_relatedSignalIndexes[i + 1])
		{
			m_relatedSignalIndexes.removeAt(i + 1);
		}
	}
	m_frameSignalIndexLimits.resize(RP_MAX_FRAME_COUNT);
	for (int i = 0; i < RP_MAX_FRAME_COUNT; i++)
	{
		std::pair<int, int> limits;
		limits.first = -1;
		limits.second = -1;
		bool firstTime = true;
		for (int j = 0; j < m_relatedSignalIndexes.size(); j++)
		{
			if (m_signalSet[m_relatedSignalIndexes[j]].regAddr().offset() >= int(RP_PACKET_DATA_SIZE * i / sizeof(m_buffer[0])))
			{
				if (m_signalSet[m_relatedSignalIndexes[j]].regAddr().offset() < int((RP_PACKET_DATA_SIZE * (i + 1) / sizeof(m_buffer[0]) + 1)))
				{
					if (firstTime)
					{
						limits.first = j;
						firstTime = false;
					}
					limits.second = j;
				}
				else
				{
					break;
				}
			}
		}
		m_frameSignalIndexLimits[i] = limits;
	}
}

quint64 SignalTableModel::getAdc(const Signal& signal) const
{
	if (signal.dataSize() > 64)
	{
		return 0;
	}
	int offset = signal.regAddr().offset();
	int bit = signal.regAddr().bit();
	int size = signal.dataSize();
	if ((offset < 0) || (offset + (bit + size) / 8 >= RP_BUFFER_SIZE))
	{
		return 0;
	}
	quint64 adc = m_buffer[offset] >> bit;
	int bitsCopied = sizeof(m_buffer[0]) * 8 - bit;
	if (bitsCopied > size)
	{
		adc &= (1ull << size) - 1ull;
		bitsCopied = size;
	}
	offset++;
	while (bitsCopied < signal.dataSize())
	{
		int bitsToRead = std::min(signal.dataSize() - bitsCopied, static_cast<int>(sizeof(m_buffer[0]) * 8));
		adc += (m_buffer[offset] & ((1ull << bitsToRead) - 1ull)) << bitsCopied;
		bitsCopied += bitsToRead;
		offset++;
	}
	return adc;
}

