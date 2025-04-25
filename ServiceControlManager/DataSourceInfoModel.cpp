#include "DataSourceInfoModel.h"
#include <CommonLib/HostAddressPort.h>

DataSourceInfoModel::DataSourceInfoModel()
{
}

int DataSourceInfoModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_rows.size());
}

int DataSourceInfoModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant DataSourceInfoModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::CheckStateRole ||
		role == Qt::DecorationRole ||
		role == Qt::EditRole)
	{
		return QVariant();
	}

	int row = index.row();
	int column = index.column();

	if (row < 0 || row >= TO_INT(m_rows.size()) ||
		column < 0 || column >= 2)
	{
		return QVariant(Separator::EMPTY_STR);
	}

	if (role == Qt::DisplayRole)
	{
		if (column == 0)
		{
			return m_rows[row];
		}

		switch (row)
		{
		case 0:	return QString::fromStdString(m_state.lmequipmentid());
		case 1: return QString::fromStdString(m_state.lmcaption());
		case 2: return QString("0x%1 (%2)").arg(m_state.moduletype(), 4, 16, Latin1Char::ZERO).arg(m_state.moduletype());
		case 3: return QString::fromStdString(m_state.modulepresetname());
		case 4: return m_state.moduleworkcyclemcs();
		case 5: return m_state.rupprotocolversion();
		case 6: return QString::fromStdString(m_state.subsystemid());
		case 7: return m_state.subsystemkey();
		case 8: return m_state.lmnumber();
		case 9: return QString::fromStdString(m_state.subsystemchannel());
		case 10: return QString::fromStdString(m_state.lancontrollerid());
		case 11: return HostAddressPort(m_state.lancontrollerip(), m_state.lancontrollerport()).toString();
		}

	/*	QString("Module EquipmentID"),					// 0

			QString("Lan controller data type"),			// 12
			QString("Expected DataUID"),					// 13
			QString("Data frames quantity"),				// 14
			QString("Data size, bytes"),					// 15*/


		return Separator::EMPTY_STR;
	}

	return QVariant("");
}

void DataSourceInfoModel::updateData(const Network::AppDataSourceState& state)
{
	m_state = state;

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}
