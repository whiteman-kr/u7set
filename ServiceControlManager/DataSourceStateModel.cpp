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

/*		QString("Receives data"),						// 0
			QString("Uptime"),								// 1
			QString("LM time"),								// 2
			QString("RUP frame numerator"),					// 3
			QString("Data receiving speed, bytes/s"),		// 4
			QString("Received data size, bytes"),			// 5
			QString("Received frames count"),				// 6
			QString("Received packet count"),				// 7
			QString("Lost packet count"),					// 8
			QString("Received RUP protocol version"),		// 9
			QString("Received DataUID"),					// 10
			QString("Signal states queue current size"),	// 11
			QString("Signal states queue MAX size"),		// 12

			QString("Error RUP protocol version"),			// 13
			QString("Error frames quantity"),				// 14
			QString("Error frame numerator"),				// 15
			QString("Error frame CRC"),						// 16
			QString("Error DataUID"),						// 17
			QString("Error duplicate plant time"),			// 18
			QString("Error non-monotonic plant time"),		// 19
			QString("Error plant time format"),				// 20*/


		switch (row)
		{
		case 0:	return (m_state.receivesdata() ? "Yes" : "No");
		case 1: return formatUptime(m_state.uptime());
		case 2: return formatTime_YYYY_MM_DD(m_state.lmtime());
//		case 3: return QString::fromStdString(m_state.modulepresetname());
//		case 4: return m_state.moduleworkcyclemcs();
/*		case 5: return m_state.rupprotocolversion();
		case 6: return QString::fromStdString(m_state.subsystemid());
		case 7: return m_state.subsystemkey();
		case 8: return m_state.lmnumber();
		case 9: return QString::fromStdString(m_state.subsystemchannel());
		case 10: return QString::fromStdString(m_state.lancontrollerid());
		case 11: return HostAddressPort(m_state.lancontrollerip(), m_state.lancontrollerport()).toString();
		case 12: return QString("0x%1 (%2)").arg(QString("%1").arg(m_state.expecteddataid(), 8, 16, Latin1Char::ZERO).toUpper()).
											arg(m_state.expecteddataid());
		case 13: return m_state.dataframesquantity();
		case 14: return m_state.datasizebytes();*/
		}


		return Separator::EMPTY_STR;
	}

	return QVariant("");
}

void DataSourceStateModel::updateData(const Network::AppDataSourceState& state)
{
	m_state = state;

	emit dataChanged(index(0, 1), index(TO_INT(m_rows.size()) - 1, 1));
}
