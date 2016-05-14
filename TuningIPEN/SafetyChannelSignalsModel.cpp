#include "SafetyChannelSignalsModel.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include <cmath>
#include <QMessageBox>

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
		Signal& signal = m_sourceInfo.tuningSignals[index.row()];
		switch (index.column())
		{
			case SIGNAL_ID_COLUMN: return signal.customAppSignalID();
			case SIGNAL_DESCRIPTION_COLUMN: return signal.caption();
			case NEW_VALUE_COLUMN:
			{
				double value = m_values[index.row()].second;
				if (qIsNaN(value))
				{
					return "";
				}
				else
				{
					if (signal.isAnalog())
					{
						return value;
					}
					else
					{
						return value == 0 ? "No" : "Yes";
					}
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
					if (signal.isAnalog())
					{
						return value;
					}
					else
					{
						return value == 0 ? "No" : "Yes";
					}
				}
			}
			break;
			case LOW_LIMIT_COLUMN: return signal.lowLimit();
			case HIGH_LIMIT_COLUMN: return signal.highLimit();
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

	Signal& signal = m_sourceInfo.tuningSignals[index.row()];

	bool ok = false;
	double newValue = value.toDouble(&ok);
	if (!ok)
	{
		if (signal.isDiscrete())
		{
			switch (value.toString().trimmed()[0].toLower().toLatin1())
			{
				case 'y':
					newValue = 1;
					break;
				case 'n':
					newValue = 0;
					break;
				default:
					QMessageBox::critical(nullptr, "Not valid input", "Please, enter 0 or 1");
					return false;
			}
		}
		else
		{
			QMessageBox::critical(nullptr, "Not valid input", "Please, enter valid float pointing number");
			return false;
		}
	}

	if (newValue < signal.lowLimit() || newValue > signal.highLimit())
	{
		QMessageBox::critical(nullptr, "Not valid input", QString("Please, enter number between %1 and %2").arg(signal.lowLimit()).arg(signal.highLimit()));
		return false;
	}

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>%1</b> signal value to <b>%2</b>?")
									   .arg(signal.customAppSignalID()).arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
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
	for (Signal& signal : m_sourceInfo.tuningSignals)
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
		if (m_values[signalIndex].second == value)
		{
			m_values[signalIndex].second = qQNaN();
			emit dataChanged(index(signalIndex, NEW_VALUE_COLUMN), index(signalIndex, NEW_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
		}

		emit dataChanged(index(signalIndex, CURRENT_VALUE_COLUMN), index(signalIndex, CURRENT_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
	}
}
