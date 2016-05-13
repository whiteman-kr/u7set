#include "SafetyChannelSignalsModel.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include <cmath>

const int	SIGNAL_ID_COLUMN = 0,
			SIGNAL_DESCRIPTION_COLUMN = 1,
			NEW_VALUE_COLUMN = 2,
			CURRENT_VALUE_COLUMN = 3,
			LOW_LIMIT_COLUMN = 4,
			HIGH_LIMIT_COLUMN = 5,
			COLUMN_COUNT = 6;

SafetyChannelSignalsModel::SafetyChannelSignalsModel(TuningDataSourceInfo& sourceInfo, TuningService* service, QObject* parent) :
	QAbstractTableModel(parent),
	m_sourceInfo(sourceInfo),
	m_service(service)
{
	m_values.resize(m_sourceInfo.tuningSignals.count());
	for (auto& pair : m_values)
	{
		pair.first = qQNaN();
		pair.second = qQNaN();
	}

	for (int i = 0; i < m_sourceInfo.tuningSignals.count(); i++)
	{
		signalIdMap.insert(m_sourceInfo.tuningSignals[i].appSignalID(), i);
	}

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &SafetyChannelSignalsModel::updateSignalStates);
	timer->start(200);
}

int SafetyChannelSignalsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m_sourceInfo.tuningSignals.count();
}

int SafetyChannelSignalsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return COLUMN_COUNT;
}

QVariant SafetyChannelSignalsModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
			case SIGNAL_ID_COLUMN: return m_sourceInfo.tuningSignals[index.row()].customAppSignalID();
			case SIGNAL_DESCRIPTION_COLUMN: return m_sourceInfo.tuningSignals[index.row()].caption();
			case NEW_VALUE_COLUMN:
			{
				double value = m_values[index.row()].second;
				if (qIsNaN(value))
				{
					return "";
				}
				else
				{
					return value;
				}
			}
			break;
			case CURRENT_VALUE_COLUMN:
			{
				double value = m_values[index.row()].first;
				if (qIsNaN(value))
				{
					return "";
				}
				else
				{
					return value;
				}
			}
			break;
			case LOW_LIMIT_COLUMN: return m_sourceInfo.tuningSignals[index.row()].lowLimit();
			case HIGH_LIMIT_COLUMN: return m_sourceInfo.tuningSignals[index.row()].highLimit();
		}
	}
	return QVariant();
}

QVariant SafetyChannelSignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
			case SIGNAL_ID_COLUMN:
				return "Signal ID";
			case SIGNAL_DESCRIPTION_COLUMN:
				return "Signal description";
			case NEW_VALUE_COLUMN:
				return "New value";
			case CURRENT_VALUE_COLUMN:
				return "Current value";
			case LOW_LIMIT_COLUMN:
				return "Low limit";
			case HIGH_LIMIT_COLUMN:
				return "High limit";
		}
	}

	return QVariant();
}

bool SafetyChannelSignalsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.column() != NEW_VALUE_COLUMN || role != Qt::EditRole)
	{
		return QAbstractTableModel::setData(index, value, role);
	}

	double newValue = value.toDouble();

	Signal& signal = m_sourceInfo.tuningSignals[index.row()];

	if (newValue < signal.lowLimit() || newValue > signal.highLimit())
	{
		return false;
	}

	m_values[index.row()].second = value.toDouble();
	emit dataChanged(index, index);

	m_service->setSignalState(signal.appSignalID(), newValue);
	return true;
}

Qt::ItemFlags SafetyChannelSignalsModel::flags(const QModelIndex& index) const
{
	if (index.column() == NEW_VALUE_COLUMN)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	return QAbstractTableModel::flags(index);
}

void SafetyChannelSignalsModel::updateSignalStates()
{
	for (auto signal : m_sourceInfo.tuningSignals)
	{
		m_service->getSignalState(signal.appSignalID());
	}
}

void SafetyChannelSignalsModel::updateSignalState(QString appSignalID, double value)
{
	if (signalIdMap.contains(appSignalID))
	{
		int signalIndex = signalIdMap[appSignalID];
		m_values[signalIndex].first = value;

		emit dataChanged(index(signalIndex, CURRENT_VALUE_COLUMN), index(signalIndex, CURRENT_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
	}
}
