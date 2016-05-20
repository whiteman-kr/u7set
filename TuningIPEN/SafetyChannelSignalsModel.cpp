#include "SafetyChannelSignalsModel.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include <cmath>
#include <QMessageBox>

const int	SIGNAL_ID_COLUMN = 0,
			SIGNAL_DESCRIPTION_COLUMN = 1,
			NEW_VALUE_COLUMN = 2,
			CURRENT_VALUE_COLUMN = 3,
			DEFAULT_VALUE_COLUMN = 4,
			ORIGINAL_LOW_LIMIT_COLUMN = 5,
			ORIGINAL_HIGH_LIMIT_COLUMN = 6,
			RECEIVED_LOW_LIMIT_COLUMN = 7,
			RECEIVED_HIGH_LIMIT_COLUMN = 8,
			COLUMN_COUNT = 9;

bool SafetyChannelSignalsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem&, const QModelIndex& index)
{
	SafetyChannelSignalsModel* signalsModel = qobject_cast<SafetyChannelSignalsModel*>(model);
	if (signalsModel == nullptr)
	{
		return false;
	}
	if (signalsModel->signal(index).isAnalog())
	{
		return false;
	}
	if (event->type() == QEvent::MouseButtonDblClick)
	{
		emit aboutToChangeDiscreteSignal(index);
		return true;
	}
	return false;
}

SafetyChannelSignalsModel::SafetyChannelSignalsModel(Tuning::TuningDataSourceInfo& sourceInfo, Tuning::TuningService* service, QObject* parent) :
	QAbstractTableModel(parent),
	m_sourceInfo(sourceInfo),
	m_service(service)
{
	m_states.resize(m_sourceInfo.tuningSignals.count());
	for (auto& state : m_states)
	{
		state.currentValue = qQNaN();
		state.newValue = qQNaN();
		state.lowLimit = qQNaN();
		state.highLimit = qQNaN();
		state.validity = false;
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
	Signal& signal = m_sourceInfo.tuningSignals[index.row()];
	auto state = m_states[index.row()];
	switch (role)
	{
		case Qt::DisplayRole:
		{
			switch (index.column())
			{
				case SIGNAL_ID_COLUMN: return signal.customAppSignalID();
				case SIGNAL_DESCRIPTION_COLUMN: return signal.caption();
				case NEW_VALUE_COLUMN:
				{
					double value = state.newValue;
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
					double value = state.currentValue;
					if (qIsNaN(value))
					{
						return "";
					}
					else
					{
						if (state.validity == false)
						{
							return "???";
						}
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
				case DEFAULT_VALUE_COLUMN: return signal.tuningDefaultValue();
				case ORIGINAL_LOW_LIMIT_COLUMN: return signal.lowEngeneeringUnits();
				case ORIGINAL_HIGH_LIMIT_COLUMN: return signal.highEngeneeringUnits();
				case RECEIVED_LOW_LIMIT_COLUMN:
				{
					double value = state.lowLimit;
					if (qIsNaN(value))
					{
						return "";
					}
					else
					{
						if (state.validity == false)
						{
							return "???";
						}
						return value;
					}
				}
				break;
				case RECEIVED_HIGH_LIMIT_COLUMN:{
					double value = state.highLimit;
					if (qIsNaN(value))
					{
						return "";
					}
					else
					{
						if (state.validity == false)
						{
							return "???";
						}
						return value;
					}
				}
				break;
			}
		}
		break;
		case Qt::BackgroundColorRole:
		{
			if (index.column() != CURRENT_VALUE_COLUMN)
			{
				return QVariant();
			}
			if (state.validity == false)
			{
				return QColor(Qt::red);
			}
			if (qAbs(state.currentValue - signal.tuningDefaultValue()) > std::numeric_limits<double>::epsilon())
			{
				return QColor(Qt::yellow);
			}
			return QVariant();
		}
		break;
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
				return "New\nvalue";
			case CURRENT_VALUE_COLUMN:
				return "Current\nvalue";
			case DEFAULT_VALUE_COLUMN:
				return "Default\nvalue";
			case ORIGINAL_LOW_LIMIT_COLUMN:
				return "Original\nLow limit";
			case ORIGINAL_HIGH_LIMIT_COLUMN:
				return "Original\nHigh limit";
			case RECEIVED_LOW_LIMIT_COLUMN:
				return "Received\nLow limit";
			case RECEIVED_HIGH_LIMIT_COLUMN:
				return "Received\nHigh limit";
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

	QString valueStr = value.toString().replace(',', '.');

	Signal& signal = m_sourceInfo.tuningSignals[index.row()];

	bool ok = false;
	double newValue = valueStr.toDouble(&ok);
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

	if (newValue < signal.lowEngeneeringUnits() || newValue > signal.highEngeneeringUnits())
	{
		QMessageBox::critical(nullptr, "Not valid input", QString("Please, enter number between %1 and %2").arg(signal.lowEngeneeringUnits()).arg(signal.highEngeneeringUnits()));
		return false;
	}

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>%1</b> signal value to <b>%2</b>?")
									   .arg(signal.customAppSignalID()).arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return false;
	}

	m_states[index.row()].newValue = newValue;
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

Signal& SafetyChannelSignalsModel::signal(const QModelIndex& index)
{
	return m_sourceInfo.tuningSignals[index.row()];
}

void SafetyChannelSignalsModel::updateSignalStates()
{
	for (Signal& signal : m_sourceInfo.tuningSignals)
	{
		m_service->getSignalState(signal.appSignalID());
	}
}

void SafetyChannelSignalsModel::updateSignalState(QString appSignalID, double value, double lowLimit, double highLimit, bool validity)
{
	QVector<int> roles;
	roles << Qt::DisplayRole;

	if (signalIdMap.contains(appSignalID))
	{
		int signalIndex = signalIdMap[appSignalID];

		m_states[signalIndex].currentValue = value;
		m_states[signalIndex].lowLimit = lowLimit;
		m_states[signalIndex].highLimit = highLimit;
		m_states[signalIndex].validity = validity;

		if (qAbs(m_states[signalIndex].newValue - value) < std::numeric_limits<double>::epsilon())
		{
			m_states[signalIndex].newValue = qQNaN();
			emit dataChanged(index(signalIndex, NEW_VALUE_COLUMN), index(signalIndex, NEW_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
		}

		emit dataChanged(index(signalIndex, CURRENT_VALUE_COLUMN), index(signalIndex, CURRENT_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
		emit dataChanged(index(signalIndex, RECEIVED_LOW_LIMIT_COLUMN), index(signalIndex, RECEIVED_HIGH_LIMIT_COLUMN), QVector<int>() << Qt::DisplayRole);
	}
}

void SafetyChannelSignalsModel::changeDiscreteSignal(const QModelIndex& index)
{
	Signal& signal = m_sourceInfo.tuningSignals[index.row()];

	if (signal.isAnalog())
	{
		assert(false);
		return;
	}

	double newValue = m_states[index.row()].currentValue == 0 ? 1 : 0;

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>%1</b> signal value to <b>%2</b>?")
									   .arg(signal.customAppSignalID()).arg(newValue == 1 ? "Yes" : "No"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_states[index.row()].newValue = newValue;
	emit dataChanged(index, index);

	m_service->setSignalState(signal.appSignalID(), newValue);
}
