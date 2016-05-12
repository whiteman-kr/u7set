#include "SafetyChannelSignalsModel.h"

SafetyChannelSignalsModel::SafetyChannelSignalsModel(QObject* parent) :
	QAbstractTableModel(parent)
{

}

int SafetyChannelSignalsModel::rowCount(const QModelIndex& parent) const
{
	return 0;
}

int SafetyChannelSignalsModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

QVariant SafetyChannelSignalsModel::data(const QModelIndex& index, int role) const
{
	return QVariant();
}

QVariant SafetyChannelSignalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{

	}

	return QVariant();		// !!!!

}
