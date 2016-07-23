#include "TripleChannelSignalsModel.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include <cmath>
#include <QMessageBox>

const int	SIGNAL_ID_COLUMN = 0,
			SIGNAL_DESCRIPTION_COLUMN = 1,
			NEW_VALUE_COLUMN = 2,
			CURRENT_VALUE_COLUMN = 3,
			DEFAULT_VALUE_COLUMN = 4,
			COLUMN_COUNT = 5;

TripleChannelSignalsDelegate::TripleChannelSignalsDelegate(QObject* parent) :
	QStyledItemDelegate(parent)
{
}

bool TripleChannelSignalsDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem&, const QModelIndex& index)
{
	TripleChannelSignalsModel* signalsModel = qobject_cast<TripleChannelSignalsModel*>(model);
	if (signalsModel == nullptr)
	{
		return false;
	}
	if (event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::KeyPress)
	{
		emit aboutToChangeDiscreteSignal(index);
		return true;
	}
	return false;
}

TripleChannelSignalsModel::TripleChannelSignalsModel(QVector<Tuning::TuningDataSourceInfo>& sourceInfo, Tuning::TuningService* service, QObject* parent) :
	QAbstractTableModel(parent),
	m_sourceInfo(sourceInfo),
	m_service(service)
{
	for (int i = 0; i < 3; i++)
	{
		m_sourceStates[i].index = -1;
	}
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &TripleChannelSignalsModel::updateSignalStates);
	timer->start(200);
}

int TripleChannelSignalsModel::rowCount(const QModelIndex&) const
{
	return m_sourceStates[0].signalStates.count();
}

int TripleChannelSignalsModel::columnCount(const QModelIndex&) const
{
	return COLUMN_COUNT;
}

QVariant TripleChannelSignalsModel::data(const QModelIndex& currentIndex, int role) const
{
	QVector<const SignalProperties*> tripleState = state(currentIndex);
	QVector<Signal*> tripleSignal = signal(currentIndex);

	switch (role)
	{
		case Qt::DisplayRole:
		{
			QString resultStr;
			for (int i = 0; i < 3; i++)
			{
				if (!resultStr.isEmpty())
				{
					resultStr += "\n";
				}
				switch (currentIndex.column())
				{
					case SIGNAL_ID_COLUMN:
						resultStr += tripleSignal[i]->customAppSignalID().trimmed();
					break;
					case SIGNAL_DESCRIPTION_COLUMN:
						resultStr += tripleSignal[i]->caption().trimmed();
					break;
					case NEW_VALUE_COLUMN:
					{
						double value = tripleState[i]->newValue;
						if (qIsNaN(value))
						{
							resultStr += " ";
						}
						else
						{
							if (tripleSignal[i]->isAnalog())
							{
								resultStr += QString::number(value);
							}
							else
							{
								resultStr += value == 0 ? "No" : "Yes";
							}
						}
					}
					break;
					case CURRENT_VALUE_COLUMN:
					{
						double value = tripleState[i]->currentValue;
						if (qIsNaN(value))
						{
							resultStr += " ";
						}
						else
						{
							if (tripleState[i]->validity == false)
							{
								resultStr += "???";
							}
							else
							{
								if (tripleSignal[i]->isAnalog())
								{
									resultStr += QString::number(value);
								}
								else
								{
									resultStr += value == 0 ? "No" : "Yes";
								}
							}
						}
					}
					break;
					case DEFAULT_VALUE_COLUMN:
					{
						double value = tripleSignal[i]->tuningDefaultValue();
						if (tripleSignal[i]->isAnalog())
						{
							resultStr += QString::number(value);
						}
						else
						{
							resultStr += value == 0 ? "No" : "Yes";
						}
					}
				}
			}
			return resultStr;
		}
		break;
		case Qt::BackgroundColorRole:
		{
			if (currentIndex.column() != CURRENT_VALUE_COLUMN)
			{
				return QVariant();
			}
			for (int i = 0; i < 3; i++)
			{
				if (tripleState[i]->validity == false)
				{
					return QColor(Qt::red);
				}
			}
			for (int i = 1; i < 3; i++)
			{
				if (qAbs(static_cast<float>(tripleState[i]->currentValue) - static_cast<float>(tripleState[i - 1]->currentValue)) > std::numeric_limits<float>::epsilon())
				{
					return QColor(Qt::red);
				}
			}
			for (int i = 0; i < 3; i++)
			{
				if (qAbs(static_cast<float>(tripleState[i]->currentValue) - static_cast<float>(tripleSignal[i]->tuningDefaultValue())) > std::numeric_limits<float>::epsilon())
					//if (data(index(currentIndex.row(), DEFAULT_VALUE_COLUMN)).toString() != data(currentIndex).toString())
				{
					return QColor(Qt::yellow);
				}
			}
			return QVariant();
		}
		break;
	}
	return QVariant();
}

QVariant TripleChannelSignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
		}
	}

	return QVariant();
}

bool TripleChannelSignalsModel::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int /*role*/)
{
	return true;
}

Qt::ItemFlags TripleChannelSignalsModel::flags(const QModelIndex& index) const
{
	if (index.column() == NEW_VALUE_COLUMN)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}
	return QAbstractTableModel::flags(index);
}

void TripleChannelSignalsModel::addTripleSignal(QVector<QString> ids)
{
	for (int i = 0; i < 3; i++)
	{
		m_idToChannelMap.insert(ids[i], i);
		auto& map = m_sourceStates[i].idToSignalIndexMap;

		if (map.contains(ids[i]))
		{
			int index = map.value(ids[i]);
			m_sourceStates[i].signalStates.push_back(SignalProperties(index));
			m_sourceStates[i].idToStateIndexMap.insert(ids[i], m_sourceStates[i].signalStates.count() - 1);
			continue;
		}

		if (m_sourceStates[i].index == -1)
		{
			//first search
			for (int infoIndex = 0; infoIndex < m_sourceInfo.count(); infoIndex++)
			{
				for (int signalIndex = 0; signalIndex < m_sourceInfo[infoIndex].tuningSignals.count(); signalIndex++)
				{
					QString signalID = m_sourceInfo[infoIndex].tuningSignals[signalIndex].appSignalID();
					if (signalID == ids[i])
					{
						map.insert(signalID, signalIndex);
						m_sourceStates[i].index = infoIndex;
						break;
					}
				}
				if (m_sourceStates[i].index != -1)
				{
					break;
				}
			}
		}
		if (m_sourceStates[i].index == -1)
		{
			assert(false);
			return;
		}
		auto& currentSourceInfo = m_sourceInfo[m_sourceStates[i].index];

		for (int signalIndex = 0; signalIndex < currentSourceInfo.tuningSignals.count(); signalIndex++)
		{
			Signal& signal = currentSourceInfo.tuningSignals[signalIndex];
			if (!map.contains(signal.appSignalID()))
			{
				map.insert(signal.appSignalID(), signalIndex);
			}

			if (signal.appSignalID() == ids[i])
			{
				m_sourceStates[i].signalStates.push_back(SignalProperties(signalIndex));
				m_sourceStates[i].idToStateIndexMap.insert(ids[i], m_sourceStates[i].signalStates.count() - 1);
				break;
			}
		}
	}
}

QVector<const SignalProperties*> TripleChannelSignalsModel::state(const QModelIndex& index) const
{
	QVector<const SignalProperties*> result;
	if (!index.isValid())
	{
		return result;
	}

	for (int i = 0; i < 3; i++)
	{
		result.append(&m_sourceStates[i].signalStates[index.row()]);
	}
	return result;
}

QVector<SignalProperties*> TripleChannelSignalsModel::state(const QModelIndex& index)
{
	QVector<SignalProperties*> result;
	if (!index.isValid())
	{
		return result;
	}

	for (int i = 0; i < 3; i++)
	{
		result.append(&m_sourceStates[i].signalStates[index.row()]);
	}
	return result;
}

QVector<Signal*> TripleChannelSignalsModel::signal(const QModelIndex& index) const
{
	QVector<Signal*> result;
	if (!index.isValid())
	{
		return result;
	}

	for (int i = 0; i < 3; i++)
	{
		int sourceIndex = m_sourceStates[i].index;
		int signalIndex = m_sourceStates[i].signalStates[index.row()].signalIndex;
		result.append(&m_sourceInfo[sourceIndex].tuningSignals[signalIndex]);
	}
	return result;
}

bool TripleChannelSignalsModel::contains(const QString& id)
{
	return m_idToChannelMap.contains(id);
}

void TripleChannelSignalsModel::updateSignalStates()
{
	for (int i = 0; i < 3; i++)
	{
		for (auto& id : m_sourceStates[i].idToSignalIndexMap.keys())
		{
			m_service->getSignalState(id);
		}
	}
}

void TripleChannelSignalsModel::updateSignalState(QString appSignalID, double value, double /*lowLimit*/, double /*highLimit*/, bool validity)
{
	QVector<int> roles;
	roles << Qt::DisplayRole;

	if (!m_idToChannelMap.contains(appSignalID))
	{
		assert(false);
		return;
	}
	int channel = m_idToChannelMap.value(appSignalID);

	if (!m_sourceStates[channel].idToStateIndexMap.contains(appSignalID))
	{
		assert(false);
		return;
	}

	int stateIndex = m_sourceStates[channel].idToStateIndexMap.value(appSignalID);
	SignalProperties& sp = m_sourceStates[channel].signalStates[stateIndex];

	sp.currentValue = value;
	sp.validity = validity;

	if (qAbs(static_cast<float>(sp.newValue) - static_cast<float>(value)) < std::numeric_limits<float>::epsilon())
		//if (data(index(signalIndex, NEW_VALUE_COLUMN)).toString() == data(index(signalIndex, CURRENT_VALUE_COLUMN)).toString())
	{
		sp.newValue = qQNaN();
		emit dataChanged(index(stateIndex, NEW_VALUE_COLUMN), index(stateIndex, NEW_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
	}

	emit dataChanged(index(stateIndex, CURRENT_VALUE_COLUMN), index(stateIndex, CURRENT_VALUE_COLUMN), QVector<int>() << Qt::DisplayRole);
}

void TripleChannelSignalsModel::changeDiscreteSignal(const QModelIndex& currentIndex)
{
	QVector<Signal*> tripleSignal = signal(currentIndex);

	QStringList idList;

	for (int i = 0; i < tripleSignal.count(); i++)
	{
		if (tripleSignal[i]->isAnalog())
		{
			assert(false);
			return;
		}
		idList.append(tripleSignal[i]->appSignalID().trimmed());
	}

	QWidget* parentWidget = dynamic_cast<QWidget*>(QObject::parent());

	auto reply = QMessageBox::question(parentWidget, "Confirmation", QString("which value must be set for (<b>%1</b>) signals?")
									   .arg(idList.join(',')),
									   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);

	if (reply == QMessageBox::Cancel)
	{
		return;
	}

	emit dataChanged(currentIndex, currentIndex);

	QVector<SignalProperties*> tripleState = state(currentIndex);

	for (int i = 0; i < tripleState.count(); i++)
	{
		tripleState[i]->newValue = reply == QMessageBox::Yes ? 1 : 0;
		m_service->setSignalState(tripleSignal[i]->appSignalID(), tripleState[i]->newValue);
	}
}
