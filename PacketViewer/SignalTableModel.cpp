#include "SignalTableModel.h"
#include "../include/Signal.h"

const int C_STR_ID = 0,
C_DESCRIPTION = 1,
C_ADC = 2,
C_VALUE = 3,
C_REG_ADDR = 4,
C_COUNT = 5;

const char* const Columns[] =
{
	"ID",
	"Description",
	"ADC",
	"Value",
	"Registration address"
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
				if (!signal.regAddr().isValid())
				{
					return "???";
				}
				int offset = signal.regAddr().offset();
				int bit = signal.regAddr().offset();
				if (signal.isAnalog())
				{
					quint16 adc = m_buffer[offset];
					if (bit != 0)
					{
						adc >>= bit;
						adc += m_buffer[offset + 1] << (16 - bit);
					}
					return QString("%1").arg(adc, 4, 16, QChar('0'));
				}
				else
				{
					return QString("%1").arg((m_buffer[offset] & (1 << bit)), 16, 2, QChar('0'));
				}
			}
			case C_VALUE:
			{
				if (!signal.regAddr().isValid())
				{
					return "???";
				}
				int offset = signal.regAddr().offset();
				int bit = signal.regAddr().offset();
				if (signal.isAnalog())
				{
					quint16 adc = m_buffer[offset];
					if (bit != 0)
					{
						adc >>= bit;
						adc += m_buffer[offset + 1] << (16 - bit);
					}
					return signal.lowLimit() + (adc - signal.lowADC()) * (signal.highLimit() - signal.lowLimit()) / (signal.highADC() - signal.lowADC());
				}
				else
				{
					return ((m_buffer[offset] & (1 << bit)) == 0) ? 0 : 1;
				}
			}
			case C_REG_ADDR: return signal.regAddr().toString();
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

