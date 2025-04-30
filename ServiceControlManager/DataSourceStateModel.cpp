#include "DataSourceStateModel.h"
#include <CommonLib/HostAddressPort.h>
#include "../UtilsLib/WUtils.h"

DataSourceStateModel::DataSourceStateModel()
{
}

int DataSourceStateModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return TO_INT(m_rows.size());
}

int DataSourceStateModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant DataSourceStateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section) {
		case 0: return "Property";
		case 1: return "Value";
		}
	}
	return QVariant();
}

QVariant DataSourceStateModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::CheckStateRole ||
		role == Qt::DecorationRole ||
		role == Qt::EditRole ||
		role == Qt::FontRole)
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
		case 0:	return (m_state.receivesdata() ? "Yes" : "No");
		case 1: return formatUptime(m_state.uptime());
		case 2: return formatTime_YYYY_MM_DD(m_state.lmtime());
		case 3: return m_state.rupframenumerator();
		case 4: return m_state.datareceivingspeed();
		case 5: return m_state.receivedframescount();
		case 6: return m_state.receivedpacketcount();
		case 7: return m_state.lostpacketcount();
		case 8: return m_state.signalstatesqueuecursize();
		case 9: return m_state.signalstatesqueuecurmaxsize();
		case 10: return QString("0x%1  (%2)").arg(QString("%1").arg(m_state.receiveddataid(), 8, 16, Latin1Char::ZERO).toUpper()).
												arg(m_state.receiveddataid());
		case 11: return m_state.errorprotocolversion();
		case 12: return m_state.errorframesquantity();
		case 13: return m_state.errorframeno();
		case 14: return m_state.errorframecrc();
		case 15: return m_state.errordataid();
		case 16: return m_state.errorduplicateplanttime();
		case 17: return m_state.errornonmonotonicplanttime();
		case 18: return m_state.errorplanttimeformat();
		}
	}

	return Separator::EMPTY_STR;
}

void DataSourceStateModel::updateData(const Network::AppDataSourceState& state)
{
	m_state = state;

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}
